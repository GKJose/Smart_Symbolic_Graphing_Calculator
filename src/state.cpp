#include <state.hxx>
#include <iostream>
#include <schemas.hpp>

#define _WS(rettype) rettype calc_state::wifi::WifiState::
#define _WIFI_CALLBACK void calc_state::wifi::

#define _AS(rettype) rettype calc_state::admin_app::AdminState::
#define _ADMIN_CALLBACK void calc_state::admin_app::

#define _JS(rettype) rettype calc_state::json::

#define _STATE(rettype) rettype calc_state::State::
#define _STATE_CALLBACK void calc_state::

/// global state used throughout the application.
using calc_state::State;
State global_state;

// Wifi connection

_WS(void) internet_connection_test(){
    // no guard needed, as pinging is thread-safe
    auto ping_res = run_async_cmd("ping 1.1.1.1 -c3").get();
    std::smatch sm;
    int ping_count = 0;
    if (std::regex_search(ping_res, sm, std::regex(R"~((\w+) received)~"))){
        ping_count = std::stoi(sm[1].str()); // Captures the number of pings received.
    }
    this->_has_internet = ping_count > 0;
}

_WS(int) db_to_percentage(int db){
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

_WS(bool) is_connected() const {
    #if ENABLE_WIFI
    return _is_connected;
    #else
    return true; 
    #endif
}

_WS(void) disconnect(){
    #if ENABLE_WIFI
    if (!connected_network.info.is_empty()){
        std::stringstream ss;
        ss << "wpa_cli -i wlan0 disable_network " << connected_network.num;
        run_async_cmd(ss.str()).get();
        ss.str(""); ss.clear();
        ss << "wpa_cli -i wlan0 delete_network " << connected_network.num;
        run_async_cmd(ss.str()).get();
        connected_network.info = OptNone;
    }
    #endif
}

_WS(bool) has_internet() const {
    return _has_internet;
}

_WS(void) obtain_ip_address(){
    if (!is_connected()) return;

    std::stringstream ss;
    _ip = run_async_cmd("ip route get 1.2.3.4 | awk '{print $7}'").get();
    _ip.erase(std::remove(_ip.begin(), _ip.end(), '\n'), _ip.end());
    ss << "nmap --iflist | grep " << _ip << "| awk '{print $3}'";
    _ips = run_async_cmd(ss.str()).get();
    _ips.erase(std::remove(_ips.begin(), _ips.end(), '\n'), _ips.end());
}

_WS(Option<std::string>) ip(){
    #if ENABLE_WIFI
    if (connected_network.info.is_empty())
        return OptNone;
    #endif
    return Option<std::string>(_ip);
}

_WS(Option<std::string>) ips(){
    #if ENABLE_WIFI
    if (connected_network.info.is_empty())
        return OptNone;
    #endif
    return Option<std::string>(_ips);
}

_WS(calc_state::wifi::WifiConnectionResult) connect(WifiNetworkInfo const& network, std::string psk){
    #if ENABLE_WIFI
    using rti = std::regex_token_iterator<std::string::iterator>;
    std::lock_guard<std::mutex> guard(connect_mutex);
    auto net_num = run_async_cmd("wpa_cli -i wlan0 add_network").get(); // returns an integer
    this->connected_network.num = std::stoi(net_num);
    this->connected_network.info = network;
    std::stringstream ss;
    ss << "wpa_cli -i wlan0 set_network " << connected_network.num << " ssid '\"" << network.ssid << "\"'";
    
    // set network ssid
    if (run_async_cmd(ss.str()).get() != "OK\n"){
        connected_network.num = -1;
        connected_network.info = OptNone;
        return ConnectionFailure;
    }
        

    if (network.has_psk){
        ss.str(""); ss.clear();
        ss << "wpa_cli -i wlan0 set_network " << this->connected_network.num << " psk '\"" << psk << "\"'";
        // set network password
        if (run_async_cmd(ss.str()).get() != "OK\n"){
            connected_network.num = -1;
            connected_network.info = OptNone;
            return ConnectionFailure;
        }
            
    }
    ss.str(""); ss.clear();
    ss << "wpa_cli -i wlan0 enable_network " << this->connected_network.num;
    if(run_async_cmd(ss.str()).get() != "OK\n"){
        connected_network.num = -1;
        connected_network.info = OptNone;
        return ConnectionFailure;
    }    
    #endif
    obtain_ip_address();
    internet_connection_test();
    if (_has_internet){
        return ConnectionSuccessWithInternet;
    }
    return ConnectionSuccessWithoutInternet;
    
}

_WS(std::vector<calc_state::wifi::WifiNetworkInfo>) scan(){
    #if ENABLE_WIFI
    const std::lock_guard<std::mutex> guard(scan_mutex); // mutex lock to prevent another thread from scanning at the same time.
    using rti = std::regex_token_iterator<std::string::iterator>;
    auto scan_res = run_async_cmd("wpa_cli -i wlan0 scan").get();
    if (scan_res != "OK\n") return decltype(scan())();
    
    auto scan_results = run_async_cmd("wpa_cli -i wlan0 scan_results").get();
    std::regex re(R"rgx((.+)\t\w+\t(-\w+)\t(\[.+\])?\t([^\t\n]+))rgx"); // regex used to capture fields in scan_results
    std::string wpa_name = "WPA2";
    rti rend;
    int submatches[] = {1, 2, 3, 4};
    rti a(scan_results.begin(), scan_results.end(), re, submatches);
    std::vector<WifiNetworkInfo> networks;
    while (a != rend){
        WifiNetworkInfo info;
        info.mac_address = *a++;
        info.connection_strength = db_to_percentage(std::stoi(*a++));
        std::string flags = *a++;
        info.has_psk = std::search(flags.begin(), flags.end(), wpa_name.begin(), wpa_name.end()) != flags.end();
        info.ssid = *a++;
        networks.push_back(std::move(info));
    }
    return std::move(networks);
    #else 
    return decltype(scan())();
    #endif
}

// Admin Application

_AS() AdminState(calc_state::wifi::WifiState& ws):ws(ws){
    
}

#if UNDEFINED_CODE_BREAK
_AS(void) connect(AdminInfo const& admin) {
#else
_AS(Option<easywsclient::WebSocket::pointer>) connect(AdminInfo& admin) {
#endif 
    const std::lock_guard<std::mutex> guard(connecting_mutex);
    auto s = easywsclient::WebSocket::from_url("ws://" + admin.ip + ":" + admin.port);
    #if UNDEFINED_CODE_BREAK
    // current_admin = admin;
    // current_admin.value_ref().socket = s;
    if (s->getReadyState() == easywsclient::WebSocket::OPEN){
        current_admin = admin;
        current_admin.value_ref().socket = s;
    } else {
        current_admin = OptNone;
    }
    return;
    #else
    if (s->getReadyState() != easywsclient::WebSocket::OPEN)
        return OptNone;
    return Option<easywsclient::WebSocket::pointer>(s);
    #endif
}

_AS(void) disconnect(){
    const std::lock_guard<std::mutex> guard(disconnecting_mutex);
    /// TODO: Impement sending disconnection JSON to admin.
    //send_data()
    if (!current_admin.is_empty() && !current_admin.value_ref().socket.is_empty()){
        current_admin.value_ref().socket.value_ref()->close();
    }
    current_admin = OptNone;
}

_AS(std::vector<calc_state::admin_app::AdminInfo>) scan(std::string const& port) {
    using admin_vec = std::vector<AdminInfo>;
    using rti = std::regex_token_iterator<std::string::iterator>;
    const std::lock_guard<std::mutex> guard(scan_mutex);
    std::regex re(R"rgx((.+)\n)rgx");
    std::stringstream ss;

    #if ENABLE_WIFI
    if (!ws.is_connected()) return admin_vec();
    #endif

    auto ip = ws.ip().value_ref();
    auto ips = ws.ips().value_ref();
    
    ss << "nmap --open -p " << port << " " << ips << " -oG - | grep \"/open\" | awk '{ print $2 }'";
    auto app_ip_res = run_async_cmd(ss.str()).get();
    const int submatches[] = { 1 };
    rti rend;
    rti a(app_ip_res.begin(), app_ip_res.end(), re, submatches);
    admin_vec vec;
    while (a != rend){
        /// TODO: Should send information json to admin.
        std::string admin_ip = *a++;
        auto info = get_admin_info(admin_ip);
        if (info.is_empty()){
            vec.push_back(AdminInfo{ admin_ip, port, "UNKNOWN", OptNone });
        } else {
            info.value_ref().ip = admin_ip;
            vec.push_back(std::move(info.value()));
        }   
    }
    return vec;
}

_AS(bool) is_connecting() {
    if (connecting_mutex.try_lock()){
        connecting_mutex.unlock();
        return true;
    }
    return false;
}

_AS(bool) is_connected() const {
    return !current_admin.is_empty() && !current_admin.value_ref_const().socket.is_empty();
}

_AS(Option<std::vector<std::string>>) get_permissions(){
    const std::lock_guard<std::mutex> guard(permission_mutex);
    /// TODO: Implement this function.
    return OptNone;
}

_AS(Option<calc_state::admin_app::AdminInfo>) get_admin_info(std::string const& ip){
    const std::lock_guard<std::mutex> guard(info_mutex);
    /// TODO: Implement this function
    return OptNone;
}

_AS(void) send_data(std::string const& data) const {
    if (ws.is_connected() 
        && !current_admin.is_empty() 
        && !current_admin.value_ref_const().socket.is_empty() 
        && current_admin.value_ref_const().socket.value()->getReadyState() == easywsclient::WebSocket::OPEN){
        current_admin.value_ref_const().socket.value()->send(data);
    }
}   

_ADMIN_CALLBACK poll_admin_app(AdminState* state) {
    /// I don't think that polling the admin app is necessary anymore.
    /// As the admin is now selected by the user.
    if (state->ws.is_connected() && state->is_connected()){
       
    }
}

_ADMIN_CALLBACK poll_websocket(AdminState* state){
    using namespace calc_state::json;
    if (state->current_admin.is_empty()) return;
    if (state->current_admin.value_ref().socket.is_empty()) return;
    

    auto ready_state = state->current_admin.value_ref().socket.value();

    if (ready_state->getReadyState() == easywsclient::WebSocket::CLOSED){
        // Make the optional socket equal to none.
        // Do not replace the admin itself with OptNone in order to try and
        // Reconnect to the admin later.
        state->current_admin.value_ref().socket = OptNone;
    } else {
        ready_state->poll();
        ready_state->dispatch([](std::string const& message){
            json::json obj = json::json::parse(message);
            if (validate(obj, schemas::connectionAdminInfoSchema)){

            } else if (validate(obj, schemas::connectionPermissionSchema)){

            } else if (validate(obj, schemas::connectionRevokeSchema)){

            } else {
                std::cout << "Unidentified JSON recieved!\n"; // spooky
            }
            std::cout << message;
        });
    }
}

// json validation

_JS(bool) validate(nlohmann::json const& obj, nlohmann::json const& schema){
    json_validator validator(nullptr, nlohmann::json_schema::default_string_format_check);
    validator.set_root_schema(schema);
    return validator.validate(obj) != nullptr;
}

// Application state

_STATE() State():ws(),as(ws){

}

_STATE(void) take_screenshot(){
    if (!ws.is_connected()) return;
    std::lock_guard<std::mutex> guard(screenshot_mutex);
    lv_img_dsc_t* snapshot = lv_snapshot_take(lv_scr_act(), LV_IMG_CF_TRUE_COLOR_ALPHA);
    std::size_t buf_size = lv_snapshot_buf_size_needed(lv_scr_act(), LV_IMG_CF_TRUE_COLOR_ALPHA);
    std::vector<unsigned char> raw_data(snapshot->data, snapshot->data + buf_size);
    std::ofstream out("image.bin");
    out.write(reinterpret_cast<const char*>(&raw_data[0]), raw_data.size()*sizeof(unsigned char));
    run_async_cmd("convert -size 320x240 -depth 8 bgra:image.bin image.bmp").get();
    lv_snapshot_free(snapshot);
}

_STATE(void) screenshot_handle(){
    if (!as.is_connected()) return;
    
    json::json data = json::ssgcData;
    data["clientIP"] = ws.ip().value();
    data["clientName"] = "UNKNOWN";
    take_screenshot();
    std::ifstream bmp("image.bmp", std::ios::in | std::ios::binary);
    std::vector<uint8_t> contents(
        (std::istreambuf_iterator<char>(bmp)), 
        std::istreambuf_iterator<char>());
    data["data"] = base64_encode(contents.data(), contents.size(), false);
    as.send_data(data.dump());
}

_STATE(bool) connect_to_admin_app(admin_app::AdminInfo& admin){
    if (!as.is_connected()){
        std::cout << "CONNECTING TO " << admin.name << " AT " << admin.ip << ":" << admin.port << "\n";
        #if UNDEFINED_CODE_BREAK
        as.connect(admin);
        return as.is_connected();
        #else 
        admin.socket = as.connect(admin);
        return !admin.socket.is_empty();
        #endif
    }
    return false;
}

_STATE_CALLBACK screenshot_cb(State* state){
    state->screenshot_handle();
}
