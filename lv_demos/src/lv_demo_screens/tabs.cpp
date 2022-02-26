#include "lvgl/examples/lv_examples.h"
#include "calculator_screens.hxx"
#include <stdio.h>
#include "lv_demos/src/lv_demo_graphing/lv_demo_graphing.hxx"
//#if LV_USE_TABVIEW && LV_BUILD_EXAMPLES

//Disable Scroll Animation Between Tabs
static void scroll_begin_event(lv_event_t * e)
{
    /*Disable the scroll animations. Triggered when a tab button is clicked */
    if(lv_event_get_code(e) == LV_EVENT_SCROLL_BEGIN) {
        lv_anim_t * a = (lv_anim_t*)lv_event_get_param(e);
        if(a)  a->time = 0;
    }
}

void lv_main_screen_tabs(void)
{
    /*Create a Tab view object*/
    lv_obj_t *tabview;
    tabview = lv_tabview_create(lv_scr_act(), LV_DIR_TOP, 20);
    lv_obj_add_event_cb(lv_tabview_get_content(tabview), scroll_begin_event, LV_EVENT_SCROLL_BEGIN, NULL);

    /*Change style*/
    lv_obj_t * tab_btns = lv_tabview_get_tab_btns(tabview);
    lv_obj_set_style_bg_color(tab_btns, lv_palette_darken(LV_PALETTE_GREY, 3), 0);
    lv_obj_set_style_text_color(tab_btns, lv_palette_lighten(LV_PALETTE_GREY, 5), 0);
    lv_obj_set_style_border_side(tab_btns, LV_BORDER_SIDE_BOTTOM, LV_PART_ITEMS | LV_STATE_CHECKED);

    /*Add 3 tabs (the tabs are page (lv_page) and can be scrolled*/
    lv_obj_t *tab1 = lv_tabview_add_tab(tabview, "Main");
    lv_obj_t *tab2 = lv_tabview_add_tab(tabview, "Graphing");
    lv_obj_t *tab3 = lv_tabview_add_tab(tabview, "Settings");

    /*Add content to the tabs*/
    main_screen_driver(tab1);

    create_graph(tab2);
    lv_obj_t* label = lv_label_create(tab3);
    lv_label_set_text(label, "Settings");

}
//#endif
