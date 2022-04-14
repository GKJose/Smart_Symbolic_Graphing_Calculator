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

using easywsclient::WebSocket;
using nlohmann::json;
using nlohmann::json_schema::json_validator;

void pollWebsocket(lv_timer_t* timer);
void pollAdminApp(lv_timer_t* timer);
void takeScreenshot(lv_timer_t* timer);
lv_obj_t * create_text(lv_obj_t * parent, const char * icon, const char * txt, lv_menu_builder_variant_t builder_variant);
lv_obj_t * create_slider(lv_obj_t * parent, const char * icon, const char * txt, int32_t min, int32_t max, int32_t val);
lv_obj_t * create_switch(lv_obj_t * parent, const char * icon, const char * txt, bool chk);
lv_obj_t * create_button(lv_obj_t * parent, const char * txt);


struct WifiNetworkInfo{
    std::string mac_address;
    std::string ssid;
    int connection_strength;
    bool has_psk;
};

enum WifiConnectionResult{
    ConnectionFailure,
    InternalConnectionFailure,
    ConnectionSuccessWithInternet,
    ConnectionSuccessWithoutInternet
};
enum Schemas{
	connectionAdminInfo,
	connectionRevoke,
	ConnectionPermission
};
using namespace schemas;
class Settings{
    using container = lv_obj_t;
    using page = lv_obj_t;
    using section = lv_obj_t;
    using wifi_mac_address = std::string;
    
    lv_obj_t *parent, *menu, *root_page, *sub_display_page, *sub_misc_page, *sub_about_page, *sub_wifi_page,*sub_name_page;
    /// maps used for gaining information relating to UI elements and network information.
    std::map<section*, std::vector<container*>> container_map;
    std::map<page*, std::vector<section*>> section_map;
    std::map<container*, WifiNetworkInfo> network_map;
    struct {Option<WifiNetworkInfo> info; int num;} connected_network;
    std::future<int> async_wifi_scan_handle, async_wifi_connect_handle, async_app_connect_handle, async_screenshot_handle;
    std::vector<WifiNetworkInfo> available_wifi_networks;
    Option<WebSocket::pointer> ws = OptNone;
	bool isConnected;
	bool isConnectingToAdmin;
	std::string ip;
	std::string ips;
	json ssgcData = R"({"ssgcType":"clientData",
					 "clientIP":"",
					 "clientName":"",
					 "data":""})"_json;
	// json connectionAdminInfoSchema = connectionAdminInfoStr;
	// json connectionRevokeSchema = connectionRevokeStr;
	// json connectionPermissionSchema = connectionPermissionStr;

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
	void connectToAdminApp(){
		async_app_connect_handle = std::async(std::launch::async, [=]{
            if (!ws.is_empty()) return 0; // If a websocket connection is already made, do not do it again.
		isConnectingToAdmin = true;
		std::stringstream ss;
        if (ip == "") {
            ip = run_async_cmd("ip route get 1.2.3.4 | awk '{print $7}'").get();
            ip.erase(std::remove(ip.begin(), ip.end(), '\n'), ip.end());
        }
		ssgcData["clientIP"] = ip;
        std::cout << "ip of device: " << ip << "\n";
        if(ips == ""){
            ss << "nmap --iflist | grep " << ip << "| awk '{print $3}'";
			auto ips_res = run_async_cmd(ss.str()).get();
            ips_res.erase(std::remove(ips_res.begin(), ips_res.end(), '\n'), ips_res.end());
			ips = ips_res;
		}
        std::cout << "ips: " << ips << "\n";
		ss.str(""); ss.clear();
		ss << "nmap --open -p 6969 " << ips << " -oG - | grep \"/open\" | awk '{ print $2 }'";
		auto app_ip_res = run_async_cmd(ss.str()).get();
        app_ip_res.erase(std::remove(app_ip_res.begin(), app_ip_res.end(), '\n'), app_ip_res.end());
        if (app_ip_res == ""){
            std::cout << "Admin App not found.\n";
            ws = OptNone;
        } else {
            std::cout << "ip of admin app: " << app_ip_res << "\n";
		    if(app_ip_res != "") ws = WebSocket::from_url("ws://"+app_ip_res+":6969");
        }
		isConnectingToAdmin = false;
		return 0;
	 });			
	}
	bool validateJSON(json& obj,int schema){
		
		json_validator validator(nullptr,nlohmann::json_schema::default_string_format_check);
		switch(schema){
			case Schemas::connectionAdminInfo:
				validator.set_root_schema(connectionAdminInfoSchema);
				return validator.validate(obj) != NULL;
				break;
			case Schemas::connectionRevoke:
				validator.set_root_schema(connectionRevokeSchema);
				return validator.validate(obj) != NULL;
				break;
			case Schemas::ConnectionPermission:
				validator.set_root_schema(connectionPermissionSchema);
				return validator.validate(obj) != NULL;
				break;
			default:
				break;
		}
		
	}
	Option<WebSocket::pointer> getWebsocket(){
		return ws;
	}
	bool isConnectedToWifi(){
		return isConnected;
	}
	bool isConnectingToAdminApp(){
		return isConnectingToAdmin;
	}
	void sendDataToAdminApp(std::string data){
		if(!ws.is_empty() && ws.value()->getReadyState() == WebSocket::OPEN){
			ws.value()->send(data);
		}
	}
	json* getssgcDataJson(){
		return &ssgcData;
	}

    void screenshot_handle(){
		if(this->getWebsocket().is_empty()) return;
        static Option<lv_img_dsc_t*> snapshot;
        if (snapshot.is_empty())
            snapshot = lv_snapshot_take(lv_scr_act(), LV_IMG_CF_TRUE_COLOR_ALPHA);
		else{
            std::size_t buf_size = lv_snapshot_buf_size_needed(lv_scr_act(), LV_IMG_CF_TRUE_COLOR_ALPHA);
			auto snapshot_v = snapshot.value();
            std::vector<unsigned char> raw_data(snapshot_v->data, snapshot_v->data + buf_size);
            std::ofstream out("image.bin");
            out.write(reinterpret_cast<const char*>(&raw_data[0]), raw_data.size()*sizeof(unsigned char));
			lv_snapshot_free(snapshot_v);
		    snapshot = OptNone;
		}
        async_screenshot_handle = std::async(std::launch::async, [=]{
            json ssgcData = this->ssgcData;
            auto poggers = run_async_cmd("convert -size 320x240 -depth 8 bgra:image.bin image.bmp").get();
            std::ifstream inny("image.bmp", std::ios::in | std::ios::binary);
            std::vector<uint8_t> contents((std::istreambuf_iterator<char>(inny)), std::istreambuf_iterator<char>());
            auto size = contents.size();
            ssgcData["data"] = base64_encode(contents.data(), contents.size(), false);
            this->sendDataToAdminApp(ssgcData.dump());
            return 0;
        });
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

    /// Currently called by a button, will eventually be called to refresh and find new networks.
    static void add_wifi_cb(lv_event_t* e){
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
            auto scan_res = run_async_cmd("wpa_cli -i wlan0 scan").get();
            if (scan_res != "OK\n") return -1;
            
            auto scan_results = run_async_cmd("wpa_cli -i wlan0 scan_results").get();
            std::regex re(R"rgx((.+)\t\w+\t(-\w+)\t(\[.+\])?\t([^\t\n]+))rgx"); // regex used to capture fields in scan_results
            std::string wpa_name = "WPA2";
            rti rend;
            int submatches[] = {1, 2, 3, 4};
            rti a(scan_results.begin(), scan_results.end(), re, submatches);
            settings->available_wifi_networks.clear();
            while (a != rend){
                WifiNetworkInfo info;
                info.mac_address = *a++;
                info.connection_strength = db_to_percentage(std::stoi(*a++));
                std::string flags = *a++;
                info.has_psk = std::search(flags.begin(), flags.end(), wpa_name.begin(), wpa_name.end()) != flags.end();
                info.ssid = *a++;
                settings->available_wifi_networks.push_back(std::move(info));
            }
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
        settings->remove_wifi_network();
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
        // !n8zW&6#b3TaSQ
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
            // std::stringstream ss;
            // ss << "Connecting to " << info.ssid;
            // lv_obj_t* connecting_msgbox = lv_msgbox_create(nullptr, "Connecting.", ss.str().c_str(), nullptr, false);
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
        if (section_map.find(sub_wifi_page) == section_map.end())
            return;
        if (section_map[sub_wifi_page].size() < 2) // wifi network has two sections
            return;
        auto sec = section_map[sub_wifi_page][1]; // wifi network section
        if (container_map.find(sec) != container_map.end()) {
            auto& con = container_map[sec]; // All wifi network containers.
            // erase all conatainers.
            while (con.size() > 0){
                lv_obj_del(con[con.size()-1]);
                con.pop_back();
            }
        }
        // create new empty containers
        for (std::size_t i = 0; i < available_wifi_networks.size(); i++){
            add_wifi_network(available_wifi_networks[i]);
        }
    }
    
    /// Disconnects the system from a wifi network.
    /// Updates the GUI to showcase the network disconnected
    void wifi_network_disconnect(){
        #if ENABLE_WIFI
        std::stringstream ss;
        ss << "wpa_cli -i wlan0 disable_network " << connected_network.num;
        run_async_cmd(ss.str()).get();
        ss.str(""); ss.clear();
        ss << "wpa_cli -i wlan0 delete_network " << connected_network.num;
        run_async_cmd(ss.str()).get();
        #endif

        if (section_map.find(sub_wifi_page) == section_map.end()) return;
        if (section_map[sub_wifi_page].size() < 2) return; // wifi network ahs two sections
        
        auto current_network_sec = section_map[sub_wifi_page][0]; // current connection section.
        if (container_map[current_network_sec].size() != 2) return; // Should be two containers in the current network section
        auto current_network_con = container_map[current_network_sec][0];
        auto network_btn_con = container_map[current_network_sec][1];
        lv_obj_t* current_network_label = lv_obj_get_child(current_network_con, 0);
        lv_obj_t* btnmat = lv_obj_get_child(network_btn_con, 0); // get the button matrix
        lv_label_set_text(current_network_label, "Not connected.");
        lv_btnmatrix_set_btn_ctrl(btnmat, 1, LV_BTNMATRIX_CTRL_HIDDEN); // hide the button to keep users from clicking it.
    }

    /// Connect the system to the given network.
    /// Updates the GUI to showcase the new network connection and internet availability.
    WifiConnectionResult wifi_network_connect(WifiNetworkInfo const& network, std::string psk){

        if (section_map.find(sub_wifi_page) == section_map.end())
            return InternalConnectionFailure;
        if (section_map[sub_wifi_page].size() < 2) // wifi network has two sections
            return InternalConnectionFailure;
         auto current_network_sec = section_map[sub_wifi_page][0]; // current connection section.
        if (container_map[current_network_sec].size() != 2) 
            return InternalConnectionFailure; // Should be two containers in the current network section
        auto current_network_con = container_map[current_network_sec][0]; 
        auto network_btn_con = container_map[current_network_sec][1];
        lv_obj_t* current_network_label = lv_obj_get_child(current_network_con, 0); // label
        lv_obj_t* btnmat = lv_obj_get_child(network_btn_con, 0); // get the button matrix

        #if ENABLE_WIFI
        auto net_num = run_async_cmd("wpa_cli -i wlan0 add_network").get(); // returns an integer
        this->connected_network.num = std::stoi(net_num);
        this->connected_network.info = network;
        std::stringstream ss;
        ss << "wpa_cli -i wlan0 set_network " << this->connected_network.num << " ssid '\"" << network.ssid << "\"'";
        
        // set network ssid
        if (run_async_cmd(ss.str()).get() != "OK\n")
            return ConnectionFailure;

        if (network.has_psk){
            ss.str(""); ss.clear();
            ss << "wpa_cli -i wlan0 set_network " << this->connected_network.num << " psk '\"" << psk << "\"'";
            // set network password
            if (run_async_cmd(ss.str()).get() != "OK\n")
                return ConnectionFailure;
        }
        ss.str(""); ss.clear();
        ss << "wpa_cli -i wlan0 enable_network " << this->connected_network.num;
        if(run_async_cmd(ss.str()).get() != "OK\n")
            return ConnectionFailure;
        #endif

        

        // test internet connectivity
        // pings a max of 3 times to check if the device is connected to the internet.
        async_wifi_connect_handle = std::async(std::launch::async, [=]{
            auto ping_res = run_async_cmd("ping 1.1.1.1 -c3").get();
            std::smatch sm;
            int ping_count = 0;
            if (std::regex_search(ping_res, sm, std::regex(R"~((\w+) received)~"))){
                ping_count = std::stoi(sm[1].str()); // Captures the number of pings received.
            }
            //lv_label_set_text_fmt(current_network_label, "Connected to %s.", network.ssid.c_str()); 
            
            lv_label_set_text_fmt(current_network_label, 
                "Connected to %s.\n%s.", 
                network.ssid.c_str(), 
                (ping_count > 0 ? "Internet Available" : "Internet Unavailable"));
				isConnected = (ping_count > 0);
            
            lv_btnmatrix_clear_btn_ctrl(btnmat, 1, LV_BTNMATRIX_CTRL_HIDDEN); // make it visible again.
            return 0;
        });
        
        return ConnectionSuccessWithInternet;
    }

    void add_wifi_network(WifiNetworkInfo const& network){
        if (section_map.find(sub_wifi_page) == section_map.end())
            return;
        if (section_map[sub_wifi_page].size() < 2) // wifi network page has two sections.
            return;
        container* con = create_container(section_map[sub_wifi_page][1]);
        network_map[con] = network; // add network to network_map
        // Add lock image if the network is password-protected
        create_text(con, network.has_psk ? (char*)&lock_icon : nullptr, network.ssid.c_str(), LV_MENU_ITEM_BUILDER_VARIANT_1);
        //lv_obj_t* percentagio = lv_label_create(con);
		if(network.connection_strength > 0 && network.connection_strength < 25){
			create_text(con,(char*)&wifi_signal_0_icon,nullptr,LV_MENU_ITEM_BUILDER_VARIANT_1);
		}else if(network.connection_strength > 25 && network.connection_strength < 50){
			create_text(con,(char*)&wifi_signal_1_icon,nullptr,LV_MENU_ITEM_BUILDER_VARIANT_1);
		}else if(network.connection_strength > 50 && network.connection_strength < 75){
			create_text(con,(char*)&wifi_signal_2_icon,nullptr,LV_MENU_ITEM_BUILDER_VARIANT_1);
		}else{
			create_text(con,(char*)&wifi_signal_3_icon,nullptr,LV_MENU_ITEM_BUILDER_VARIANT_1);
		}
        //lv_label_set_text_fmt(percentagio, "%d%%", network.connection_strength);
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
            network_map.erase(container_map[sec][con.size()-1]); // remove from network_map
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
		 async_wifi_connect_handle = std::async(std::launch::async, [=]{
            auto ping_res = run_async_cmd("ping 1.1.1.1 -c3").get();
            std::smatch sm;
            int ping_count = 0;
            if (std::regex_search(ping_res, sm, std::regex(R"~((\w+) received)~"))){
                ping_count = std::stoi(sm[1].str()); // Captures the number of pings received.
            }
			lv_label_set_text_fmt(label, "%s",(ping_count > 0 ? "Internet Available" : "Internet Unavailable"));
			isConnected = (ping_count > 0);
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
        lv_obj_add_event_cb(btnmat, add_wifi_cb, LV_EVENT_VALUE_CHANGED, this);

        create_section(sub_wifi_page); 
        container* wifi_con = create_root_text_container(sub_wifi_page, LV_SYMBOL_WIFI, "Wifi");
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
    lv_timer_create(pollAdminApp, 5000, &settings);
	lv_timer_create(pollWebsocket,250,&settings);
	lv_timer_create(takeScreenshot,500,&settings);
    #if ENABLE_MCP_KEYPAD
    softPwmCreate(5,100,100);
    softPwmWrite(5,100);
    #endif
	
}

void pollAdminApp(lv_timer_t* timer){
    Settings* settings = (Settings*)timer->user_data;
    auto ws = settings->getWebsocket();
    if (settings->isConnectedToWifi() && !settings->isConnectingToAdminApp() && ws.is_empty()){
        settings->connectToAdminApp();
    }
}
void takeScreenshot(lv_timer_t* timer){
	Settings* settings = (Settings*)timer->user_data;
    settings->screenshot_handle();
}
void pollWebsocket(lv_timer_t* timer){
	Settings* settings = (Settings*)timer->user_data;
	auto ws = settings->getWebsocket();
    if (ws.is_empty()) return;
    
    if (ws.value()->getReadyState() == WebSocket::CLOSED){
        // make it a None optional value to make it attempt to reconnect to admin server.
        ws = OptNone; 
    } else if (ws.value()->getReadyState() != WebSocket::CLOSED){
        ws.value()->poll();
        ws.value()->dispatch([](const std::string & message){
            json obj = json::parse(message.c_str());
            printf(message.c_str());
        });
    }
    
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
