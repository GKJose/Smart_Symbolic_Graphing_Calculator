#include "lvgl/examples/lv_examples.h"
#include "calculator_screens.h"
#include <stdio.h>
#include "lv_demos/src/lv_demo_graphing/lv_demo_graphing.hxx"
#if LV_USE_TABVIEW && LV_BUILD_EXAMPLES

void lv_main_screen_tabs(void)
{
    /*Create a Tab view object*/
    lv_obj_t *tabview;
    tabview = lv_tabview_create(lv_scr_act(), LV_DIR_TOP, 20);
    /*Add 3 tabs (the tabs are page (lv_page) and can be scrolled*/
    lv_obj_t *tab1 = lv_tabview_add_tab(tabview, "Main");
    lv_obj_t *tab2 = lv_tabview_add_tab(tabview, "Graphing");
    lv_obj_t *tab3 = lv_tabview_add_tab(tabview, "Settings");
    //printf("%f", height);
    
    /*Add content to the tabs*/
    main_screen_driver(tab1);
    /*lv_label_set_text(label, "This the first tab\n\n"
                             "If the content\n"
                             "of a tab\n"
                             "becomes too\n"
                             "longer\n"
                             "than the\n"
                             "container\n"
                             "then it\n"
                             "automatically\n"
                             "becomes\n"
                             "scrollable.\n"
                             "\n"
                             "\n"
                             "\n"
                             "Can you see it?");*/

    //lv_obj_t* label = lv_label_create(tab2);
    //lv_label_set_text(label, "Second tab");
    //create_graph();
    lv_obj_t* label = lv_label_create(tab3);
    lv_label_set_text(label, "Third tab");

    lv_obj_scroll_to_view_recursive(label, LV_ANIM_ON);

}
#endif
