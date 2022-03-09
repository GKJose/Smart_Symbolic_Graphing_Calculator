#include "Settings.hxx"
#include <iostream>

//Menu declarations
static lv_obj_t * menu;
static lv_obj_t * cont;
static lv_obj_t * section;

static lv_obj_t * root_page;
static lv_obj_t * sub_brightness_page;
static lv_obj_t * sub_software_info_page;
static lv_obj_t * sub_about_page;

static lv_obj_t * brightnessSlider;



static void slider_event_cb(lv_event_t * e)
{

    lv_obj_t * slider = lv_event_get_target(e);
    #if ENABLE_PI
    softPwmWrite(5,(int)lv_slider_get_value(slider));
	std::cout << (int)lv_slider_get_value(slider) + "\n";
    #endif
	
	
    
}
void createSettingsTab(lv_obj_t* parent){
  //Created menu, added callback to menu, centered menu
  menu = lv_menu_create(parent);
  lv_menu_set_mode_root_back_btn(menu,LV_MENU_ROOT_BACK_BTN_ENABLED);
  lv_obj_add_event_cb(menu,back_event_handler,LV_EVENT_CLICKED,menu);
  lv_obj_set_size(menu,LV_HOR_RES,LV_VER_RES);
  lv_obj_center(menu);
  
  /* create sub pages */
  sub_brightness_page = lv_menu_page_create(menu,NULL);
  lv_obj_set_style_pad_hor(sub_brightness_page,lv_obj_get_style_pad_left(lv_menu_get_main_header(menu),0),0);
  lv_menu_separator_create(sub_brightness_page);
  section = lv_menu_section_create(sub_brightness_page);
  brightnessSlider = create_slider(section,LV_SYMBOL_SETTINGS,"Brightness",25,100,100);
  lv_obj_add_event_cb(brightnessSlider, slider_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

  sub_software_info_page = lv_menu_page_create(menu,NULL);
  lv_obj_set_style_pad_hor(sub_software_info_page, lv_obj_get_style_pad_left(lv_menu_get_main_header(menu), 0), 0);
  section = lv_menu_section_create(sub_software_info_page);
  create_text(section, NULL, "BETA. UHCL Group 2", LV_MENU_ITEM_BUILDER_VARIANT_1);

  sub_about_page = lv_menu_page_create(menu,NULL);
  lv_obj_set_style_pad_hor(sub_about_page, lv_obj_get_style_pad_left(lv_menu_get_main_header(menu), 0), 0);  
  lv_menu_separator_create(sub_about_page);  
  section = lv_menu_section_create(sub_about_page);
  cont = create_text(section, NULL, "Software information: Beta Version", LV_MENU_ITEM_BUILDER_VARIANT_1);
  lv_menu_set_load_page_event(menu, cont, sub_software_info_page);	

	
  /*create a root page */
    root_page = lv_menu_page_create(menu, "Settings");
    lv_obj_set_style_pad_hor(root_page, lv_obj_get_style_pad_left(lv_menu_get_main_header(menu), 0), 0);
    section = lv_menu_section_create(root_page);
	
    cont = create_text(section, LV_SYMBOL_SETTINGS, "Brightness", LV_MENU_ITEM_BUILDER_VARIANT_1);
    lv_menu_set_load_page_event(menu, cont, sub_brightness_page);
	
    cont = create_text(section, NULL, "About", LV_MENU_ITEM_BUILDER_VARIANT_1);
    lv_menu_set_load_page_event(menu, cont, sub_about_page);
    
    lv_menu_set_sidebar_page(menu, root_page);

    lv_event_send(lv_obj_get_child(lv_obj_get_child(lv_menu_get_cur_sidebar_page(menu), 0), 0), LV_EVENT_CLICKED, NULL);  
  
    
  #if ENABLE_PI
  softPwmCreate(5,100,100);
  softPwmWrite(5,100);

  #endif
  
  

}
static void back_event_handler(lv_event_t * e)
{
    lv_obj_t * obj = lv_event_get_target(e);

    if(lv_menu_back_btn_is_root(menu, obj)) {
        lv_obj_t * mbox1 = lv_msgbox_create(NULL, "Hello", "Root back btn click.", NULL, true);
        lv_obj_center(mbox1);
    }
}

static void switch_handler(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = lv_event_get_target(e);
    if(code == LV_EVENT_VALUE_CHANGED) {
        if(lv_obj_has_state(obj, LV_STATE_CHECKED)) {
            lv_menu_set_page(menu, NULL);
            lv_menu_set_sidebar_page(menu, root_page);
            lv_event_send(lv_obj_get_child(lv_obj_get_child(lv_menu_get_cur_sidebar_page(menu), 0), 0), LV_EVENT_CLICKED, NULL);
        }
        else {
            lv_menu_set_sidebar_page(menu, NULL);
            lv_menu_clear_history(menu); /* Clear history because we will be showing the root page later */
            lv_menu_set_page(menu, root_page);
        }
    }
}
static lv_obj_t * create_text(lv_obj_t * parent, const char * icon, const char * txt,
                              lv_menu_builder_variant_t builder_variant)
{
    lv_obj_t * obj = lv_menu_cont_create(parent);

    lv_obj_t * img = NULL;
    lv_obj_t * label = NULL;

    if(icon) {
        img = lv_img_create(obj);
        lv_img_set_src(img, icon);
    }

    if(txt) {
        label = lv_label_create(obj);
        lv_label_set_text(label, txt);
        lv_label_set_long_mode(label, LV_LABEL_LONG_SCROLL_CIRCULAR);
        lv_obj_set_flex_grow(label, 1);
    }

    if(builder_variant == LV_MENU_ITEM_BUILDER_VARIANT_2 && icon && txt) {
        lv_obj_add_flag(img, LV_OBJ_FLAG_FLEX_IN_NEW_TRACK);
        lv_obj_swap(img, label);
    }

    return obj;
}

static lv_obj_t * create_slider(lv_obj_t * parent, const char * icon, const char * txt, int32_t min, int32_t max,
                                int32_t val)
{
    lv_obj_t * obj = create_text(parent, icon, txt, LV_MENU_ITEM_BUILDER_VARIANT_2);

    lv_obj_t * slider = lv_slider_create(obj);
    lv_obj_set_flex_grow(slider, 1);
    lv_slider_set_range(slider, min, max);
    lv_slider_set_value(slider, val, LV_ANIM_OFF);

    if(icon == NULL) {
        lv_obj_add_flag(slider, LV_OBJ_FLAG_FLEX_IN_NEW_TRACK);
    }

    return slider;
}

static lv_obj_t * create_switch(lv_obj_t * parent, const char * icon, const char * txt, bool chk)
{
    lv_obj_t * obj = create_text(parent, icon, txt, LV_MENU_ITEM_BUILDER_VARIANT_1);

    lv_obj_t * sw = lv_switch_create(obj);
    lv_obj_add_state(sw, chk ? LV_STATE_CHECKED : 0);

    return obj;
}
