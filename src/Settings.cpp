#include <Settings.hxx>
#include <iostream>
#include <map>
#include <vector>
#include <sstream>
#include <set>
#include <lock_icon_15x17.hxx>
#include <option.hxx>

lv_obj_t * create_text(lv_obj_t * parent, const char * icon, const char * txt, lv_menu_builder_variant_t builder_variant);
lv_obj_t * create_slider(lv_obj_t * parent, const char * icon, const char * txt, int32_t min, int32_t max, int32_t val);
lv_obj_t * create_switch(lv_obj_t * parent, const char * icon, const char * txt, bool chk);
lv_obj_t * create_button(lv_obj_t * parent, const char * txt);

struct WifiNetworkInfo{
    std::string ssid;
    int connection_strength;
    bool has_psk;
};

class Settings{
    using container = lv_obj_t;
    using page = lv_obj_t;
    using section = lv_obj_t;
    lv_obj_t *parent, *menu, *root_page, *sub_display_page, *sub_misc_page, *sub_about_page, *sub_wifi_page;
    std::map<section*, std::vector<container*>> container_map;
    std::map<page*, std::vector<section*>> section_map;
    Option<WifiNetworkInfo> connected_network;
    std::set<WifiNetworkInfo> available_wifi_networks;

    public:

    Settings(lv_obj_t* parent):parent(parent),menu(lv_menu_create(parent)){
        lv_menu_set_mode_root_back_btn(menu,LV_MENU_ROOT_BACK_BTN_DISABLED);
        lv_obj_add_event_cb(menu,back_event_handler,LV_EVENT_CLICKED,menu);
        lv_obj_set_size(menu,LV_HOR_RES,LV_VER_RES - 20);
        lv_obj_center(menu);

        lv_color_t bg_color = lv_obj_get_style_bg_color(menu, 0);
        lv_obj_set_style_bg_color(menu, lv_color_darken(bg_color, 10), 0);

        init_root_page();
        init_display_page();
        init_misc_page();
        init_about_page();
        init_wifi_page();

        lv_event_send(lv_obj_get_child(lv_obj_get_child(lv_menu_get_cur_sidebar_page(menu), 0), 0), LV_EVENT_CLICKED, nullptr);  
    }

    private:

    static void back_event_handler(lv_event_t * e){}

    static void slider_event_cb(lv_event_t * e){
        lv_obj_t * slider = lv_event_get_target(e);
        #if ENABLE_MCP_KEYPAD
        softPwmWrite(5,(int)lv_slider_get_value(slider));
        std::cout << (int)lv_slider_get_value(slider) + "\n";
        #endif 
    }

    static void updateButton_cb(lv_event_t * e){
        lv_event_code_t code = lv_event_get_code(e);
        if(code == LV_EVENT_CLICKED){
            lv_obj_t * mbox1 = lv_msgbox_create(lv_obj_get_parent(lv_event_get_target(e)), "Info.", "Updating in progress.", NULL, true);
            lv_obj_center(mbox1);
            
        }
    }

    static void add_wifi_cb(lv_event_t* e){
        static int count = 0;
        LV_ASSERT(e->user_data != nullptr);
        std::stringstream ss;
        ss << "network " << count++;
        Settings* settings = static_cast<Settings*>(e->user_data);
        settings->add_wifi_network(ss.str(), count % 2 == 0);
    }

    static void remove_wifi_cb(lv_event_t* e){
        LV_ASSERT(e->user_data != nullptr);
        Settings* settings = static_cast<Settings*>(e->user_data);
        settings->remove_wifi_network();
    }

    static void wifi_msgbox_cb(lv_event_t* e){
        lv_obj_t* popup = lv_msgbox_create(nullptr, "Connect to network", "Howdy pardner", nullptr, true);
        lv_obj_center(popup);
    }

    void add_wifi_network(std::string name, bool has_psk){
        if (section_map.find(sub_wifi_page) == section_map.end())
            return;
        if (section_map[sub_wifi_page].size() < 2) // wifi network page has two sections.
            return;
        container* con = create_container(section_map[sub_wifi_page][1]);
        create_text(con, has_psk ? (char*)&lock_icon : nullptr, name.c_str(), LV_MENU_ITEM_BUILDER_VARIANT_1);
        lv_obj_add_flag(con, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_event_cb(con, wifi_msgbox_cb, LV_EVENT_CLICKED, this);
    }

    void remove_wifi_network(){
        if (section_map.find(sub_wifi_page) == section_map.end())
            return;
        if (section_map[sub_wifi_page].size() < 2) // wifi network has two sections
            return;
        auto sec = section_map[sub_wifi_page][1];
        if (container_map.find(sec) == container_map.end())
            return;
        auto& con = container_map[sec];
        if (con.size() > 0){
            lv_obj_del(container_map[sec][con.size()-1]);
            container_map[sec].pop_back();
        }
        
    }

    page* init_page(){
        LV_ASSERT(menu!=nullptr);
        page* obj = lv_menu_page_create(menu, nullptr);
        lv_obj_set_style_pad_hor(obj, lv_obj_get_style_pad_left(lv_menu_get_main_header(menu),0),0);
        lv_menu_separator_create(obj);
        return obj;
    }

    section* create_section(page* p){
        section* obj = lv_menu_section_create(p);
        if (section_map.find(p) == section_map.end()){
            section_map[p] = std::vector<section*>{obj};
        } else {
            section_map[p].push_back(obj);
        }
        return obj;
    }

    container* create_container(section* p){
        container* obj = lv_menu_cont_create(p);
        if (container_map.find(p) == container_map.end()){
            container_map[p] = std::vector<container*>{obj};
        } else {
            container_map[p].push_back(obj);
        }
        return obj;
    }

    void create_root_text_container(page* p, const char* icon, const char* name){
        LV_ASSERT(root_page!=nullptr);
        LV_ASSERT(section_map.find(root_page) != section_map.end());
        LV_ASSERT(section_map[root_page].size() > 0);
        container* rc = create_container(section_map[root_page][0]);
        lv_obj_t* rc_text = create_text(rc, icon, name, LV_MENU_ITEM_BUILDER_VARIANT_1);
        lv_menu_set_load_page_event(menu, rc, p);
    }

    void init_root_page(){
        root_page = lv_menu_page_create(menu, (char*)"Settings");
        lv_obj_set_style_pad_hor(root_page, lv_obj_get_style_pad_left(lv_menu_get_main_header(menu), 0), 0);
        section* sec = create_section(root_page);
        lv_menu_set_sidebar_page(menu, root_page);
    }

    void init_display_page(){
        sub_display_page = init_page();
        section* sec = create_section(sub_display_page);
        container* con = create_container(sec);
        lv_obj_t* slider = create_slider(con, LV_SYMBOL_SETTINGS, "Brightness", 25, 100, 100);
        lv_obj_add_event_cb(slider, slider_event_cb, LV_EVENT_VALUE_CHANGED, nullptr);
        create_root_text_container(sub_display_page, LV_SYMBOL_IMAGE, "Display");
    }

    void init_misc_page(){
        sub_misc_page = init_page();
        section* sec = create_section(sub_misc_page);
        lv_obj_t* button = create_button(sub_misc_page, "Update");
        lv_obj_add_event_cb(button, updateButton_cb, LV_EVENT_ALL, nullptr);
        create_root_text_container(sub_misc_page, LV_SYMBOL_REFRESH, "Misc");
    }

    void init_about_page(){
        sub_about_page = init_page();
        section* sec = create_section(sub_about_page);
        container* con = create_container(sec);
        lv_obj_t* text = create_text(con, nullptr, "Software Version: Alpha 1", LV_MENU_ITEM_BUILDER_VARIANT_1);
        create_root_text_container(sub_about_page, LV_SYMBOL_WARNING, "About");
    }

    void init_wifi_page(){
        sub_wifi_page = init_page();
        section* sec = create_section(sub_wifi_page);
        container* con = create_container(sec);
        lv_obj_t* text = create_text(con, nullptr, "Hello, this is a test", LV_MENU_ITEM_BUILDER_VARIANT_1);
        create_section(sub_wifi_page); // test
        lv_obj_t* add_b = create_button(sub_wifi_page, "+");
        lv_obj_t* remove_b = create_button(sub_wifi_page, "-");
        lv_obj_add_event_cb(add_b, add_wifi_cb, LV_EVENT_CLICKED, this);
        lv_obj_add_event_cb(remove_b, remove_wifi_cb, LV_EVENT_CLICKED, this);
        create_root_text_container(sub_wifi_page, LV_SYMBOL_WIFI, "Wifi");
    }
};

void createSettingsTab(lv_obj_t* parent){
    
    static Settings settings(parent);

    #if ENABLE_MCP_KEYPAD
    softPwmCreate(5,100,100);
    softPwmWrite(5,100);
    #endif
}

// static void switch_handler(lv_event_t * e)
// {
//     lv_event_code_t code = lv_event_get_code(e);
//     lv_obj_t * obj = lv_event_get_target(e);
//     if(code == LV_EVENT_VALUE_CHANGED) {
//         if(lv_obj_has_state(obj, LV_STATE_CHECKED)) {
//             lv_menu_set_page(menu, NULL);
//             lv_menu_set_sidebar_page(menu, root_page);
//             lv_event_send(lv_obj_get_child(lv_obj_get_child(lv_menu_get_cur_sidebar_page(menu), 0), 0), LV_EVENT_CLICKED, NULL);
//         }
//         else {
//             lv_menu_set_sidebar_page(menu, NULL);
//             lv_menu_clear_history(menu); /* Clear history because we will be showing the root page later */
//             lv_menu_set_page(menu, root_page);
//         }
//     }
// }

lv_obj_t * create_text(lv_obj_t * parent, const char * icon, const char * txt,
                              lv_menu_builder_variant_t builder_variant)
{
    lv_obj_t* obj = parent;
    
    
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

lv_obj_t * create_slider(lv_obj_t * parent, const char * icon, const char * txt, int32_t min, int32_t max,
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

lv_obj_t * create_switch(lv_obj_t * parent, const char * icon, const char * txt, bool chk)
{
    lv_obj_t * obj = create_text(parent, icon, txt, LV_MENU_ITEM_BUILDER_VARIANT_1);

    lv_obj_t * sw = lv_switch_create(obj);
    lv_obj_add_state(sw, chk ? LV_STATE_CHECKED : 0);

    return obj;
}

lv_obj_t * create_button(lv_obj_t * parent, const char * txt){
	lv_obj_t * label;
	
	lv_obj_t * btn = lv_btn_create(parent);
	label = lv_label_create(btn);
	lv_label_set_text(label,txt);
	lv_obj_set_flex_grow(label, 1);
	
	return btn;
}
