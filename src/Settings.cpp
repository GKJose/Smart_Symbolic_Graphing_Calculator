
#include <Settings.hxx>
#include <iostream>
#include <map>
#include <vector>
#include <sstream>
#include <set>
#include <lock_icon_15x17.hxx>
#include <wifi_signal_0.hxx>
#include <wifi_signal_1.hxx>
#include <wifi_signal_2.hxx>
#include <wifi_signal_3.hxx>
#include <option.hxx>
#include <async_extensions.hxx>
#include <regex>
#include <lvgl.h>
#include <lvgl/src/core/lv_event.h>
#include <Calculator.h>
#include <easywsclient.hpp>
#include <stdio.h>
#include <nlohmann/json.hpp>
#include <sstream>
#include <nlohmann/json-schema.hpp>
#include <fstream>
#include <schemas.hpp>
#include <base64.h>
#include <state.hxx>
#include <tuple>

using easywsclient::WebSocket;
using nlohmann::json;
using nlohmann::json_schema::json_validator;

// void pollWebsocket(lv_timer_t* timer);
// void pollAdminApp(lv_timer_t* timer);
// void takeScreenshot(lv_timer_t* timer);
lv_obj_t * create_text(lv_obj_t * parent, const char * icon, const char * txt, lv_menu_builder_variant_t builder_variant);
lv_obj_t * create_slider(lv_obj_t * parent, const char * icon, const char * txt, int32_t min, int32_t max, int32_t val);
lv_obj_t * create_switch(lv_obj_t * parent, const char * icon, const char * txt, bool chk);
lv_obj_t * create_button(lv_obj_t * parent, const char * txt);

using namespace schemas;
void* setting;
class Settings{
    using container = lv_obj_t;
    using page = lv_obj_t;
    using section = lv_obj_t;
    using wifi_mac_address = std::string;
    using WifiNetworkInfo = calc_state::wifi::WifiNetworkInfo;
    using AdminInfo = calc_state::admin_app::AdminInfo;
    using WifiConnectionResult = calc_state::wifi::WifiConnectionResult;
    template<typename T>
    using update_map = std::map<container*, T>;
    
    lv_obj_t *parent, *menu, *root_page, *sub_display_page, *sub_misc_page, *sub_about_page, *sub_wifi_page, *sub_admin_page, *sub_name_page;
    /// maps used for gaining information relating to UI elements and network information.
    std::map<section*, std::vector<container*>> container_map;
    std::map<page*, std::vector<section*>> section_map;
    update_map<WifiNetworkInfo> network_map;
    update_map<AdminInfo> admin_map;
    struct {Option<WifiNetworkInfo> info; int num;} connected_network;
    std::future<int> async_wifi_scan_handle, async_wifi_connect_handle, async_app_connect_handle, async_screenshot_handle;
    std::vector<WifiNetworkInfo> available_wifi_networks;
    // Option<WebSocket::pointer> ws = OptNone;
	// bool isConnected;
	// bool isConnectingToAdmin;
	// std::string ip;
	// std::string ips;
	// json ssgcData = R"({"ssgcType":"clientData",
	// 				 "clientIP":"",
	// 				 "clientName":"",
	// 				 "data":""})"_json;

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
        init_admin_page();

        lv_event_send(lv_obj_get_child(lv_obj_get_child(lv_menu_get_cur_sidebar_page(menu), 0), 0), LV_EVENT_CLICKED, nullptr);  
        setting = this;
    }
    friend void forceDisconnect();
    void delete_admin_buttons(){
        auto second_sec = section_map[sub_admin_page][1];
        std::string text = "Connected to an admin.";
        admin_map.clear();
        for(auto button:container_map[second_sec]){
            lv_obj_del(button);
        }
        container_map.erase(second_sec);

        auto first_sec = section_map[sub_admin_page][0]; 
        auto first_con = container_map[first_sec][0];
        auto second_con = container_map[first_sec][1];

        lv_obj_t* first_label = lv_obj_get_child(first_con, 0);
        lv_obj_t* btnmat = lv_obj_get_child(second_con, 0);
        lv_label_set_text(first_label, text.c_str());
        lv_btnmatrix_set_btn_ctrl(btnmat, 0, LV_BTNMATRIX_CTRL_HIDDEN);

    }
    // void screenshot_handle(){
    //     async_screenshot_handle = std::async(std::launch::async, [=]{
    //         global_state.screenshot_handle();
    //         return 0;
    //     });
    // }

    /// Disconnects from the currently-connected administrator.
    /// Updates the GUI to reflect the disconnection.
    void admin_disconnect(){
        auto first_sec = section_map[sub_admin_page][0]; 
        auto first_con = container_map[first_sec][0];
        auto second_con = container_map[first_sec][1];

        global_state.as.disconnect();

        lv_obj_t* first_label = lv_obj_get_child(first_con, 0);
        lv_obj_t* btnmat = lv_obj_get_child(second_con, 0);
        delete_admin_buttons();
        lv_label_set_text(first_label, "Admin session search.");
        lv_btnmatrix_set_btn_ctrl(btnmat, 1, LV_BTNMATRIX_CTRL_HIDDEN);
        lv_btnmatrix_clear_btn_ctrl(btnmat, 0,LV_BTNMATRIX_CTRL_HIDDEN);
        //update_admin_page();
      
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

    static int db_to_percentage(int db){
        // Algorithm taken from: 
        // https://github.com/torvalds/linux/blob/9ff9b0d392ea08090cd1780fb196f36dbb586529/drivers/net/wireless/intel/ipw2x00/ipw2200.c#L4322
        constexpr int perfect_rssi = -20;
        constexpr int worst_rssi = -85;
        int nominal_rssi = perfect_rssi - worst_rssi;
        int signal_quality = (100 * nominal_rssi * nominal_rssi 
        - (perfect_rssi - db) * (15 * nominal_rssi + 62*(perfect_rssi - db)))
        / (nominal_rssi * nominal_rssi);
        if (signal_quality > 100){
            signal_quality = 100;
        } else if (signal_quality < 0){
            signal_quality = 0;
        }
        return signal_quality;
    }

    /// Refreshes and finds new networks.
    static void wifi_scan_cb(lv_event_t* e){
        using rti = std::regex_token_iterator<std::string::iterator>;
        static int count = 0;
        LV_ASSERT(e->user_data != nullptr);
        uint16_t id = lv_btnmatrix_get_selected_btn(e->target);
        Settings* settings = static_cast<Settings*>(e->user_data);
        std::stringstream ss;
        if (id == 1){ // disconnect
            settings->wifi_network_disconnect();
            return;
        }
        // scan

        // -- will be replaced with actual network
        ss << "network " << count++;

        // steps to scan for networks
        // wpa_cli -i wlan0 scan
        // get network info using regex: (.+)\t\w+\t(-\w+)\t(\[.+\])+\t([^\t\n]+)
        
        #if ENABLE_WIFI
        // asynchronously gather network info.
        settings->async_wifi_scan_handle = std::async(std::launch::async, [=]{
            settings->available_wifi_networks = global_state.ws.scan();
            settings->update_wifi_networks(); // show new networks in the gui.
            return 0;
        });

        #else 
        WifiNetworkInfo info;
        info.mac_address = "10:20:30:40:50:60";
        info.ssid = ss.str();
        info.connection_strength = 92;
        info.has_psk = count % 2 == 0;
        settings->available_wifi_networks.push_back(std::move(info));
        settings->update_wifi_networks();
        #endif
        // --
        
    }

    static void remove_wifi_cb(lv_event_t* e){
        LV_ASSERT(e->user_data != nullptr);
        Settings* settings = static_cast<Settings*>(e->user_data);
        settings->generic_remove(settings->sub_wifi_page);
    }

    static void wifi_msgbox_cb(lv_event_t* e){
        using WifiPair = struct {Settings* settings; lv_obj_t* textarea; lv_obj_t* con;};
        static WifiPair s; // fine because there is ever only ever one wifi message box on the screen.
        static const char* connect_text[] = {"Connect", ""};
        s.settings = (Settings*)e->user_data;
        s.con = (lv_obj_t*)e->target;
        if (!(s.settings->network_map[e->target].has_psk)){
            s.settings->wifi_network_connect(s.settings->network_map[e->target], "");
            return; // If there is no password, there is no need to show the popup, just connect.
        }
            
        std::stringstream ss;
        ss << "Connect to " << s.settings->network_map[e->target].ssid;
        lv_msgbox_t* popup = (lv_msgbox_t*)lv_msgbox_create(nullptr, ss.str().c_str(), "Enter Password:", nullptr, true);
        lv_obj_t* textarea = lv_textarea_create((lv_obj_t*)popup);
        lv_textarea_set_one_line(textarea, true);
        lv_textarea_set_password_mode(textarea, true);
        lv_obj_set_width(textarea, (popup->obj.coords.x2 - popup->obj.coords.x1) - 25);
        Calculator::storeWifiTA(textarea);
        
       
        s.textarea = textarea;
        // wifi connect button
        popup->btns = lv_btnmatrix_create((lv_obj_t*)popup);
        lv_btnmatrix_set_map(popup->btns, connect_text);
        lv_btnmatrix_set_btn_ctrl_all(popup->btns, LV_BTNMATRIX_CTRL_CLICK_TRIG | LV_BTNMATRIX_CTRL_NO_REPEAT);
        lv_obj_add_event_cb(popup->btns, [](lv_event_t* e){
            WifiPair* wp = (WifiPair*)e->user_data;
            WifiNetworkInfo const& info = wp->settings->network_map[wp->con]; // get WifiNetworkInfo associated with the container
            std::string text(lv_textarea_get_text(wp->textarea));
            lv_msgbox_close(lv_obj_get_parent(wp->textarea)); // close the message box
            // connect to network with the password entered into the textarea
            wp->settings->wifi_network_connect(info, text);
        }, LV_EVENT_CLICKED, &s);

        const lv_font_t * font = lv_obj_get_style_text_font(popup->btns, LV_PART_ITEMS);
        lv_coord_t btn_h = lv_font_get_line_height(font) + LV_DPI_DEF / 10;
        lv_obj_set_size(popup->btns, (2 * LV_DPI_DEF / 3), btn_h);
        lv_obj_set_style_max_width(popup->btns, lv_pct(100), 0);
        lv_obj_add_flag(popup->btns, LV_OBJ_FLAG_EVENT_BUBBLE);

        lv_obj_center((lv_obj_t*)popup);
    }

    /// Updates visible wifi networks in the GUI.
    void update_wifi_networks(){
        static auto connect_strength_within = [](int lower, int upper, WifiNetworkInfo const& network){
            return network.connection_strength > lower && network.connection_strength <= upper;
        };
        generic_update<WifiNetworkInfo>(sub_wifi_page, network_map, available_wifi_networks, 
        [=](container* con, WifiNetworkInfo const& network){
             create_text(con, network.has_psk ? (char*)&lock_icon : nullptr, network.ssid.c_str(), LV_MENU_ITEM_BUILDER_VARIANT_1);
            if (connect_strength_within(0, 25, network)){
                create_text(con,(char*)&wifi_signal_0_icon,nullptr,LV_MENU_ITEM_BUILDER_VARIANT_1);
            } else if (connect_strength_within(25, 50, network)){
                create_text(con,(char*)&wifi_signal_1_icon,nullptr,LV_MENU_ITEM_BUILDER_VARIANT_1);
            } else if (connect_strength_within(50, 75, network)){
                create_text(con,(char*)&wifi_signal_2_icon,nullptr,LV_MENU_ITEM_BUILDER_VARIANT_1);
            } else if (connect_strength_within(75, 100, network)){
                create_text(con,(char*)&wifi_signal_3_icon,nullptr,LV_MENU_ITEM_BUILDER_VARIANT_1);
            }
        },
        wifi_msgbox_cb);
    }

    template<typename T>
    void generic_update(
        lv_obj_t* page,
        std::map<container*, T>& m,
        std::vector<T> const& data,
        std::function<void(container*, T const&)> listitem_update_f,
        lv_event_cb_t listitem_press_cb){
        auto sec = section_map[page][1]; // list section;
        if (container_map.find(sec) != container_map.end()){
            auto& con = container_map[sec]; // vector of all containers in list.
            while (con.size() > 0){
                lv_obj_del(con[con.size()-1]);
                con.pop_back();
            }
        }
        // populate list
        for (std::size_t i = 0; i < data.size(); i++){
            generic_add<T>(page, m, data[i], listitem_update_f, listitem_press_cb);
        }
    }
    
    /// Disconnects the system from a wifi network.
    /// Updates the GUI to showcase the network disconnected
    void wifi_network_disconnect(){
        generic_disconnect<true>(
            sub_wifi_page, 
            [=](){global_state.ws.disconnect();},
            "Not connected.");
    }
 
    template<bool IsBlocking>
    void generic_disconnect(
        lv_obj_t* page, 
        std::function<void(void)> disconnect_backend_f,
        const char disconnect_text[]){
        auto first_sec = section_map[page][0]; 
        auto first_con = container_map[first_sec][0];
        auto second_con = container_map[first_sec][1];
        if (IsBlocking){
            disconnect_backend_f();
        } else {
            std::thread(disconnect_backend_f).detach();
        }
        lv_obj_t* first_label = lv_obj_get_child(first_con, 0);
        lv_obj_t* btnmat = lv_obj_get_child(second_con, 0);
        lv_label_set_text(first_label, disconnect_text);
        lv_btnmatrix_set_btn_ctrl(btnmat, 1, LV_BTNMATRIX_CTRL_HIDDEN);
    }

    /// Connect the system to the given network.
    /// Updates the GUI to showcase the new network connection and internet availability.
    WifiConnectionResult wifi_network_connect(WifiNetworkInfo const& network, std::string psk){
        using connect_tuple = std::tuple<lv_obj_t*, lv_obj_t*>;

        generic_connect<connect_tuple>(sub_wifi_page,
        [=](lv_obj_t* sec){
            auto current_network_con = container_map[sec][0];
            auto network_btn_con = container_map[sec][1];
            lv_obj_t* current_network_label = lv_obj_get_child(current_network_con, 0); // label
            lv_obj_t* btnmat = lv_obj_get_child(network_btn_con, 0); // get the button matrix
            return std::make_tuple(current_network_label, btnmat);
        },
        [=](connect_tuple v){
            global_state.ws.connect(network, psk);
            lv_obj_t* current_network_label, *btnmat;
            std::tie(current_network_label, btnmat) = v;
            lv_label_set_text_fmt(current_network_label, 
                "Connected to %s.\n%s.", 
                network.ssid.c_str(), 
                (global_state.ws.has_internet() ? "Internet Available" : "Internet Unavailable"));
            
            lv_btnmatrix_clear_btn_ctrl(btnmat, 1, LV_BTNMATRIX_CTRL_HIDDEN); // make it visible again.
        });

        return WifiConnectionResult::ConnectionSuccessWithInternet;
    }


    template<typename T>
    int generic_connect(
        lv_obj_t* page, 
        std::function<T(section*)> pre_thread_f,
        std::function<void(T)> thread_f){

        section* current_sec = section_map[page][0];    // first section
        
        T res = pre_thread_f(current_sec);

        std::thread([=]{
            thread_f(res);
        }).detach();

        return 0;
    }

    template<typename MapVal>
    void generic_add(
        lv_obj_t* page, 
        std::map<container*, MapVal>& m, 
        MapVal const& val, 
        std::function<void(container*, MapVal const&)> post_f,
        lv_event_cb_t press_callback){
     
        container* con = create_container(section_map[page][1]);
        m[con] = val; // add item to map.
        
        post_f(con, val);
        lv_obj_add_flag(con, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_event_cb(con, press_callback, LV_EVENT_CLICKED, this);
    }

    void generic_remove(lv_obj_t* page){
        if (section_map.find(page) == section_map.end())
            return;
        if (section_map[page].size() < 2) // wifi network has two sections
            return;
        auto sec = section_map[page][1];
        if (container_map.find(sec) == container_map.end())
            return;
        auto& con = container_map[sec];
        if (con.size() > 0){
            network_map.erase(container_map[sec][con.size()-1]); // remove from network_map
            lv_obj_del(container_map[sec][con.size()-1]);
            container_map[sec].pop_back();
        }
    }

    static void admin_scan_cb(lv_event_t* e){
        using rti = std::regex_token_iterator<std::string::iterator>;
        static int count = 0;
        LV_ASSERT(e->user_data != nullptr);
        uint16_t id = lv_btnmatrix_get_selected_btn(e->target);
        Settings* settings = static_cast<Settings*>(e->user_data);

        if (id == 1){ // disconnect button pressed
            settings->admin_disconnect();
        } else { // scan button pressed
            section* first_sec = settings->section_map.at(settings->sub_admin_page).at(0);
            container* ta_con = settings->container_map.at(first_sec).at(2); // textarea container
            std::string name = lv_textarea_get_text(lv_obj_get_child(ta_con, 1));
            if (name == "")
                name = "UNKNOWN";
            global_state.as.set_device_name(std::move(name));

            settings->update_admin_page();
        }
    }

    static void admin_msgbox_cb(lv_event_t* e){
        using AdminPair = struct {Settings* settings; lv_obj_t* list; AdminInfo* admin_info; lv_obj_t* popup; lv_obj_t* name_textarea;};
        static AdminPair ap;
        Settings* settings = (Settings*)e->user_data;
        ap.admin_info = &settings->admin_map[(container*)e->target]; 
        ap.settings = settings;
        ap.admin_info->port = global_state.port;
        static const char* connect_text[] = {"Accept", "Decline", ""};

        section* first_sec = settings->section_map.at(settings->sub_admin_page).at(0);
        container* ta_con = settings->container_map.at(first_sec).at(2); // textarea container

        lv_obj_t* name_ta = lv_obj_get_child(ta_con, 1); // name textarea
        ap.name_textarea = name_ta;

        lv_msgbox_t* popup = (lv_msgbox_t*)lv_msgbox_create(
            nullptr, 
            ap.admin_info->name.c_str(), 
            "Accept the following to connect:", 
            nullptr, 
            true);
        ap.popup = (lv_obj_t*)popup;
        
        ap.list = lv_list_create((lv_obj_t*)popup);
        lv_obj_set_size(ap.list, 220, 100);
        global_state.connect_to_admin_app(*ap.admin_info);
        auto permissions = global_state.as.get_permissions();

        if (!permissions.value_ref().empty()){
            for (auto const& perm : permissions.value_ref_const()){
                lv_list_add_text(ap.list, perm.c_str());
            }
        }
        

        popup->btns = lv_btnmatrix_create((lv_obj_t*)popup);
        
        lv_btnmatrix_set_map(popup->btns, connect_text);
        lv_btnmatrix_set_btn_width(popup->btns,0,20);
        lv_btnmatrix_set_btn_width(popup->btns,1,20);

        lv_btnmatrix_set_btn_ctrl_all(popup->btns, LV_BTNMATRIX_CTRL_CLICK_TRIG | LV_BTNMATRIX_CTRL_NO_REPEAT);
        lv_obj_add_event_cb(popup->btns, [](lv_event_t* e){
            uint16_t id = lv_btnmatrix_get_selected_btn(e->target);
            AdminPair* ap = (AdminPair*)e->user_data;
            auto s = global_state.as.get_current_admin().value_ref().socket.value_ref();
            Settings* settings = ap->settings;

            std::string name = lv_textarea_get_text(ap->name_textarea);
            if (name == "")
                name = "UNKNOWN";

            global_state.as.set_device_name(std::move(name));

            if(id == 0){
                auto reply = calc_state::json::permissionAcceptReply;
                reply["clientIP"] = "127.0.0.1";
                global_state.as.send_data(reply.dump());
                s->poll();
                global_state.set_screenshot_timer();
                global_state.set_websocket_timer();
                settings->delete_admin_buttons();
                
                
            }else{
                auto reply = calc_state::json::permissionRejectReply;
                reply["clientIP"] = "127.0.0.1";
                global_state.as.send_data(reply.dump()); 
                settings->admin_disconnect();
            }
            lv_msgbox_close(ap->popup);

            lv_obj_t* connection_result_popup = lv_msgbox_create(
                nullptr, 
                id == 0 ? "Connection Successful" : "Connection Failure",
                id == 0 ? "Successfully connected to an administrator." : "Unsuccessfully connected to an adminstrator.",
                nullptr,
                true);
            lv_obj_center(connection_result_popup); 
            auto admin_sec = settings->section_map[settings->sub_admin_page][0];
            auto current_admin_con = settings->container_map[admin_sec][0];
            auto admin_btn_con = settings->container_map[admin_sec][1];
            lv_obj_t* current_network_label = lv_obj_get_child(current_admin_con, 0); // label
            lv_obj_t* btnmat = lv_obj_get_child(admin_btn_con, 0); // get the button matrix
            if (id == 0){
                // Make the disconnect button visible if connection was successful
                lv_btnmatrix_clear_btn_ctrl(btnmat, 1, LV_BTNMATRIX_CTRL_HIDDEN);
            }
        }, LV_EVENT_CLICKED, &ap);

        const lv_font_t * font = lv_obj_get_style_text_font(popup->btns, LV_PART_ITEMS);
        lv_coord_t btn_h = lv_font_get_line_height(font) + LV_DPI_DEF / 10;
        lv_obj_set_size(popup->btns, (2 * LV_DPI_DEF / 3), btn_h);
        lv_obj_set_style_max_width(popup->btns, lv_pct(100), 0);
        lv_obj_add_flag(popup->btns, LV_OBJ_FLAG_EVENT_BUBBLE);

        lv_obj_center((lv_obj_t*)popup);
    }

    /// Tries to search for administrators visible to the device.
    /// Asynchronously displays the results on the GUI.
    void update_admin_page(){

        auto sec = section_map[sub_admin_page][0];
        auto con = container_map[sec][2]; // textarea container
        lv_obj_t* port_textarea = lv_obj_get_child(con, 0);
        std::string* str = new std::string(lv_textarea_get_text(port_textarea));
        if (str->size() > 0){
            try {
                // if this passes then the ip is indeed a number.
                std::stoul(*str);
            } catch (...){
                lv_obj_t* msgbox = lv_msgbox_create(nullptr, "ERROR", "Invalid port number.", nullptr, true);
                lv_obj_center(msgbox);
                delete str;
                return;
            }
        }
        global_state.port = str->size() > 0 ? *str : "6969";
        std::thread([=]{
            generic_update<AdminInfo>(
                sub_admin_page,
                admin_map,
                global_state.as.scan(global_state.port), // this is blocking, but because this is in a new thread it doesn't matter.
                [=](container* con, AdminInfo const& admin){
                    create_text(con, nullptr, admin.name.c_str(), LV_MENU_ITEM_BUILDER_VARIANT_1);
                    create_text(con, nullptr, admin.ip.c_str(), LV_MENU_ITEM_BUILDER_VARIANT_1);
                },
                admin_msgbox_cb
                );
                delete str; // thread has to clean up
            }).detach();
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

    container* create_root_text_container(page* p, const char* icon, const char* name){
        LV_ASSERT(root_page!=nullptr);
        LV_ASSERT(section_map.find(root_page) != section_map.end());
        LV_ASSERT(section_map[root_page].size() > 0);
        container* rc = create_container(section_map[root_page][0]);
        lv_obj_t* rc_text = create_text(rc, icon, name, LV_MENU_ITEM_BUILDER_VARIANT_1);
        lv_menu_set_load_page_event(menu, rc, p);
        return rc;
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
        static const char* btnmat_map[3] = {"Scan", "Disconnect", ""};
        sub_wifi_page = init_page();
        section* sec = create_section(sub_wifi_page);
        container* con = create_container(sec);
        lv_obj_t* label = lv_label_create(con);
        lv_label_set_text_fmt(label, "%s",(global_state.ws.has_internet() ? "Internet Available" : "Internet Unavailable"));
        async_wifi_connect_handle = std::async(std::launch::async, [=]{
                global_state.ws.internet_connection_test();
                lv_label_set_text_fmt(label, "%s",(global_state.ws.has_internet() ? "Internet Available" : "Internet Unavailable"));
                return 0;
        });
        lv_label_set_long_mode(label, LV_LABEL_LONG_SCROLL_CIRCULAR);
        lv_obj_set_flex_grow(label, 1);

        container* con2 = create_container(sec);
        lv_obj_t* btnmat = lv_btnmatrix_create(con2);
        static lv_style_t btnmat_style_bg, btnmat_style;

        lv_style_init(&btnmat_style_bg);
        lv_style_set_pad_all(&btnmat_style_bg, 10);
        lv_style_set_border_width(&btnmat_style_bg, 0);
        lv_style_set_bg_opa(&btnmat_style_bg, LV_OPA_0);

        lv_style_init(&btnmat_style);
        lv_style_set_border_opa(&btnmat_style, LV_OPA_50);
        lv_style_set_text_font(&btnmat_style, &lv_font_montserrat_12_subpx);

        lv_btnmatrix_set_map(btnmat, btnmat_map);
        lv_btnmatrix_set_btn_ctrl_all(btnmat, LV_BTNMATRIX_CTRL_CLICK_TRIG);
        lv_btnmatrix_set_btn_ctrl(btnmat, 1, LV_BTNMATRIX_CTRL_HIDDEN);
        lv_btnmatrix_set_btn_width(btnmat, 1, 2);
        lv_obj_add_style(btnmat, &btnmat_style_bg, 0);
        lv_obj_add_style(btnmat, &btnmat_style, LV_PART_ITEMS);
        lv_obj_set_size(btnmat, 150, 50);
        lv_obj_add_event_cb(btnmat, wifi_scan_cb, LV_EVENT_VALUE_CHANGED, this);

        create_section(sub_wifi_page); 
        container* wifi_con = create_root_text_container(sub_wifi_page, LV_SYMBOL_WIFI, "Wifi");
    }

    void init_admin_page(){
        static const char* btnmat_map[3] = {"Scan", "Disconnect", ""};
        sub_admin_page = init_page();
        section* sec = create_section(sub_admin_page);
        container* con = create_container(sec);
        lv_obj_t* label = lv_label_create(con);
        lv_label_set_text(label, "Admin session search.");

        container* btn_con = create_container(sec);
        lv_obj_t* btnmat = lv_btnmatrix_create(btn_con);
        
        container* port_con = create_container(sec);
        lv_obj_t* port_textarea = lv_textarea_create(port_con);
        lv_obj_t* name_textarea = lv_textarea_create(port_con);

        static lv_style_t btnmat_style_bg, btnmat_style;
        // background style
        lv_style_init(&btnmat_style_bg);
        lv_style_set_pad_all(&btnmat_style_bg, 10);
        lv_style_set_border_width(&btnmat_style_bg, 0);
        lv_style_set_bg_opa(&btnmat_style_bg, LV_OPA_0);
        // foreground style
        lv_style_init(&btnmat_style);
        lv_style_set_border_opa(&btnmat_style, LV_OPA_50);
        lv_style_set_text_font(&btnmat_style, &lv_font_montserrat_12_subpx);
        // button matrix setup
        lv_btnmatrix_set_map(btnmat, btnmat_map);
        lv_btnmatrix_set_btn_ctrl_all(btnmat, LV_BTNMATRIX_CTRL_CLICK_TRIG);
        lv_btnmatrix_set_btn_ctrl(btnmat, 1, LV_BTNMATRIX_CTRL_HIDDEN);
        lv_btnmatrix_set_btn_width(btnmat, 1, 2);
        lv_obj_add_style(btnmat, &btnmat_style_bg, 0);
        lv_obj_add_style(btnmat, &btnmat_style, LV_PART_ITEMS);
        lv_obj_set_size(btnmat, 150, 50);
        lv_obj_add_event_cb(btnmat, admin_scan_cb, LV_EVENT_VALUE_CHANGED, this);
        // port textarea setup
        lv_textarea_set_one_line(port_textarea, true);
        lv_textarea_set_placeholder_text(port_textarea, "PORT #");
        lv_obj_set_flex_grow(port_textarea, true);
        // name textarea setup
        lv_textarea_set_one_line(name_textarea, true);
        lv_textarea_set_placeholder_text(name_textarea, "NAME");
        lv_obj_set_flex_grow(name_textarea, true);

        create_section(sub_admin_page); // section to populate with admin info.
        container* admin_con = create_root_text_container(sub_admin_page, LV_SYMBOL_TRASH, "Admin");
    }

	void init_name_page(){
		sub_name_page = init_page();
		section* sec = create_section(sub_name_page);
		container* con = create_container(sec);
		create_root_text_container(sub_name_page, LV_SYMBOL_WARNING, "Name");
	}

};
void createSettingsTab(lv_obj_t* parent){
    
    static Settings settings(parent);
    #if ENABLE_MCP_KEYPAD
    softPwmCreate(5,100,100);
    softPwmWrite(5,100);
    #endif
	
}
void forceDisconnect(){
    Settings* settings = (Settings*)setting;
    settings->admin_disconnect();
}
    
// }
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
//             lv_menu_set_page(menu,dump()
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
