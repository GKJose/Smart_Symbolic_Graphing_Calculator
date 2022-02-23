#include "lvgl/examples/lv_examples.h"
#if LV_USE_TEXTAREA && LV_BUILD_EXAMPLES

//Declarations
lv_obj_t* lv_textarea_input(void);
void lv_textarea_output(void);
static void kb_event_cb(lv_event_t* e);
static void toggle_kb_event_handler(lv_event_t* e);
//void keyboard(lv_obj_t* ta);
void main_screen_driver(void);

lv_obj_t* areas[100];       
int total = 0;
static lv_obj_t* kb;
static lv_obj_t* toggle_kb_btn;

void main_screen_driver(void)
{
    /*Create a keyboard*/
    kb = lv_keyboard_create(lv_scr_act());
    lv_obj_set_size(kb, LV_HOR_RES, LV_VER_RES / 3);
    //lv_keyboard_set_textarea(kb, first_ta); /*Focus it on one of the text areas to start*/
    lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_event_cb(kb, kb_event_cb, LV_EVENT_ALL, kb);


    lv_obj_t* first_ta = lv_textarea_input();
    toggle_kb_btn = lv_btn_create(lv_scr_act());
    lv_obj_add_flag(toggle_kb_btn,LV_OBJ_FLAG_CHECKABLE);
    lv_obj_align(toggle_kb_btn, LV_ALIGN_TOP_RIGHT, 0, 0);
    lv_color_t grey = lv_palette_main(LV_PALETTE_GREY);
    lv_obj_set_style_bg_color(toggle_kb_btn, grey, 0);
    lv_obj_set_size(toggle_kb_btn, 20, 20);
    lv_obj_t* kb_img = lv_img_create(toggle_kb_btn);
    lv_img_set_src(kb_img, LV_SYMBOL_KEYBOARD);
    lv_obj_align_to(kb_img, NULL, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_event_cb(toggle_kb_btn, toggle_kb_event_handler, LV_EVENT_ALL, toggle_kb_btn);

    //(toggle_kb_btn, LV_SYMBOL_KEYBOARD, LV_PART_MAIN);

}

//Callback functions
static void textarea_event_handler(lv_event_t* e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t* ta = lv_event_get_target(e);
    if (code == LV_EVENT_CLICKED || code == LV_EVENT_FOCUSED) {
        /*Focus on the clicked text area*/
        if (kb != NULL) lv_keyboard_set_textarea(kb, ta);
    }

    else if (code == LV_EVENT_READY) {
        LV_LOG_USER("Ready, current text: %s", lv_textarea_get_text(ta));
        for (int i = 0; i < total; i++) {
            lv_obj_set_y(areas[i], lv_obj_get_y_aligned(areas[i]) - 70);
        }
        lv_textarea_output();
        lv_textarea_input();
    }

}

static void kb_event_cb(lv_event_t* e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t* target = lv_event_get_target(e);
    if (code == LV_EVENT_CANCEL) {
        lv_keyboard_set_textarea(kb, NULL);
        lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);
    }
    //printf("%d", lv_obj_get_height(kb));
}

static void toggle_kb_event_handler(lv_event_t* e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_VALUE_CHANGED) {
        if (lv_obj_has_flag(kb, LV_OBJ_FLAG_HIDDEN)) {
            lv_obj_clear_flag(kb, LV_OBJ_FLAG_HIDDEN);
            for (int i = 0; i < total; i++) {
                lv_obj_set_y(areas[i], lv_obj_get_y_aligned(areas[i]) - 90);
                //lv_indev_reset(NULL, ta);
            }
        }
        else {
            lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);
            for (int i = 0; i < total; i++) {
                lv_obj_set_y(areas[i], lv_obj_get_y_aligned(areas[i]) + 90);
                //lv_indev_reset(NULL, ta);
            }
        }

    }
    lv_obj_move_foreground(toggle_kb_btn);
}

/* Example buttons
static void btnm_event_handler(lv_event_t* e)
{
    lv_obj_t* obj = lv_event_get_target(e);
    lv_obj_t* ta = lv_event_get_user_data(e);
    const char* txt = lv_btnmatrix_get_btn_text(obj, lv_btnmatrix_get_selected_btn(obj));

    if (strcmp(txt, LV_SYMBOL_BACKSPACE) == 0) lv_textarea_del_char(ta);
    else if (strcmp(txt, LV_SYMBOL_NEW_LINE) == 0) lv_event_send(ta, LV_EVENT_READY, NULL);
    else lv_textarea_add_text(ta, txt);

}
*/

//Graphical widget functions
lv_obj_t* lv_textarea_input(void)
{
    lv_obj_t* ta = lv_textarea_create(lv_scr_act());
    areas[total] = ta;
    total++;
    lv_textarea_set_one_line(ta, true);
    lv_obj_set_width(ta, 320);
    lv_obj_align(ta, LV_ALIGN_BOTTOM_MID, 0, 10);
    lv_obj_add_event_cb(ta, textarea_event_handler, LV_EVENT_ALL, NULL);
    lv_obj_add_state(ta, LV_STATE_DEFAULT);
    if (!lv_obj_has_flag(kb, LV_OBJ_FLAG_HIDDEN)) {
        lv_obj_set_y(ta, lv_obj_get_y_aligned(ta) - 90);
    }
    return ta;

}

void lv_textarea_output(void)
{
    lv_obj_t* ta = lv_textarea_create(lv_scr_act());
    areas[total] = ta;
    total++;
    char str[50];
    sprintf(str, "Answer: %d", total/2);
    lv_textarea_set_one_line(ta, true);
    lv_obj_set_width(ta, 320);
    lv_obj_align(ta, LV_ALIGN_BOTTOM_MID, 0, 10);
    lv_obj_add_event_cb(ta, textarea_event_handler, LV_EVENT_READY, NULL);
    lv_obj_add_state(ta, LV_STATE_DEFAULT); /*To be sure the cursor is visible*/
    lv_obj_set_style_text_align(ta, LV_TEXT_ALIGN_RIGHT, 0);
    lv_textarea_add_text(ta, str);
    lv_obj_set_y(ta, lv_obj_get_y_aligned(ta) - 35);
    if (!lv_obj_has_flag(kb, LV_OBJ_FLAG_HIDDEN)) {
        lv_obj_set_y(ta, lv_obj_get_y_aligned(ta) - 90);
    }

}

/*   Trash ?
void keyboard(lv_obj_t* ta)
{
    /*Create a keyboard
    kb = lv_keyboard_create(lv_scr_act());
    lv_obj_set_size(kb, LV_HOR_RES, LV_VER_RES / 3);
    lv_obj_add_event_cb(kb, kb_event_cb, LV_EVENT_ALL, kb);
    lv_keyboard_set_textarea(kb, ta); /*Focus it on one of the text areas to star
}*/

#endif
