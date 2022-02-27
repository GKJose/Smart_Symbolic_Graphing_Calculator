#include "calculator_screens.hxx"
#include "tabs.hxx"
#include <stdio.h>
//#if LV_USE_TEXTAREA && LV_BUILD_EXAMPLES

/*Declarations*/
lv_obj_t* lv_textarea_input(lv_obj_t* parent);
lv_obj_t* lv_textarea_output(lv_obj_t* parent);
static void kb_event_cb(lv_event_t* e);
static void toggle_kb_event_handler(lv_event_t* e);
void main_screen_driver(lv_obj_t* parent);

/*Areas holds a list of pointers to the active text areas.
  (1 Input, 1 Output) = 1 Entry to the calculator.
  After 50 Entries the main screen clears to prevent memory overfill.
  Segmentation Fault occurs at 60 Entries, 120 Working Text areas.*/
lv_obj_t* areas[100];
int total;

static lv_obj_t* kb;
static lv_obj_t* toggle_kb_btn;


void main_screen_driver(lv_obj_t* parent)
{

    total = 0;
    /*Create a keyboard*/
    kb = lv_keyboard_create(parent);
    lv_obj_set_size(kb, LV_HOR_RES, LV_VER_RES / 3);
    lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_event_cb(kb, kb_event_cb, LV_EVENT_ALL, kb);

    /*Initial Input text area*/
    lv_obj_t* first_ta = lv_textarea_input(parent);
    lv_keyboard_set_textarea(kb, first_ta); /*Focus it on one of the text areas to start*/

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
    lv_obj_add_event_cb(toggle_kb_btn, toggle_kb_event_handler, LV_EVENT_ALL, toggle_kb_btn);

    /*Put kb in view*/
    lv_obj_align(kb, LV_ALIGN_BOTTOM_MID, 0, 35);

}

/*Callback functions*/
static void textarea_event_handler(lv_event_t* e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t* ta = lv_event_get_target(e);
    lv_obj_t* parent = lv_obj_get_parent(ta);
    if (code == LV_EVENT_CLICKED || code == LV_EVENT_FOCUSED)
    {
        /*Focus on the clicked text area*/
        if (kb != NULL) lv_keyboard_set_textarea(kb, ta);
    }
    else if (code == LV_EVENT_READY)
    {
        if(total > 100)
        {
            for(uint32_t i = 0; i < lv_obj_get_child_cnt(parent); i++)
            {
                lv_obj_clean(parent);
                main_screen_driver(parent);
                return;
            }
        }
        LV_LOG_USER("Ready, current text: %s", lv_textarea_get_text(ta));
        
        /*Create the new text areas*/
        lv_textarea_output(parent);
        lv_textarea_input(parent);
        
        /*Put kb in view*/
        lv_obj_align_to(kb, parent, LV_ALIGN_BOTTOM_MID, 0, 10);
    }

}


static void kb_event_cb(lv_event_t* e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t* target = lv_event_get_target(e);
    if (code == LV_EVENT_CANCEL)
    {
        lv_keyboard_set_textarea(kb, NULL);
        lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);
    }

}


static void toggle_kb_event_handler(lv_event_t* e)
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
lv_obj_t* lv_textarea_input(lv_obj_t* parent)
{
    lv_obj_t* ta = lv_textarea_create(parent);
    areas[total] = ta;
    total++;
    lv_textarea_set_one_line(ta, true);
    lv_obj_set_width(ta, 320);
    lv_obj_align(ta, LV_ALIGN_BOTTOM_MID, 0, ( 35 * total));
    lv_obj_add_event_cb(ta, textarea_event_handler, LV_EVENT_ALL, NULL);
    lv_obj_add_state(ta, LV_STATE_DEFAULT);
    
    /*Make sure the most recent Input area is in view at the bottom*/
    lv_obj_scroll_to_view(areas[total - 1], LV_ANIM_OFF);
    lv_obj_scroll_by(parent, 0, 10, LV_ANIM_OFF);
    if (!lv_obj_has_flag(kb, LV_OBJ_FLAG_HIDDEN))
    {
        lv_obj_set_y(ta, lv_obj_get_y_aligned(ta) - 80);
    }
    return ta;

}

lv_obj_t* lv_textarea_output(lv_obj_t* parent)
{
    lv_obj_t* ta = lv_textarea_create(parent);
    areas[total] = ta;
    total++;
    char str[50];
    sprintf(str, "Answer: %d", total/2);
    lv_textarea_set_one_line(ta, true);
    lv_obj_set_width(ta, 320);
    lv_obj_align(ta, LV_ALIGN_BOTTOM_MID, 0, ( 35 * total));
    lv_obj_add_event_cb(ta, textarea_event_handler, LV_EVENT_READY, NULL);
    lv_obj_add_state(ta, LV_STATE_DEFAULT); /*To be sure the cursor is visible*/
    lv_obj_set_style_text_align(ta, LV_TEXT_ALIGN_RIGHT, 0);
    lv_textarea_add_text(ta, str);
    if (!lv_obj_has_flag(kb, LV_OBJ_FLAG_HIDDEN))
    {
        lv_obj_set_y(ta, lv_obj_get_y_aligned(ta) - 80);
    }
    return ta;
}

//#endif
