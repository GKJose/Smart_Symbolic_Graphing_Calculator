#ifndef STATE_H
#define STATE_H

#include <future>
#include <vector>
#include <option.hxx>
#include <async_extensions.hxx>
#include <regex>
#include <lvgl.h>
#include <lvgl/src/core/lv_event.h>
#include <easywsclient.hpp>
#include <stdio.h>
#include <nlohmann/json.hpp>
#include <sstream>
#include <nlohmann/json-schema.hpp>
#include <fstream>
#include <schemas.hpp>
#include <base64.h>
#include <thread>
#include <mutex>
#include <calc_conf.h>


/// NOTE: None of the data structures & functions implemented below
/// are asynchronous. Every call is blocking. Async is handled by the caller.
namespace calc_state{

    namespace wifi{

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

        using easywsclient::WebSocket;

        class WifiState {
            bool _is_connected, _has_internet;
            std::string _ip;
            std::string _ips;
            struct {Option<WifiNetworkInfo> info; int num;} connected_network;
            std::mutex scan_mutex, connect_mutex;

            
            /// If connected to a network, this method sets _ip and _ips
            /// To values.
            /// ip is set to the device's ip on local network.
            /// _ips is set to the ip with a subnet.
            void obtain_ip_address();
            int db_to_percentage(int db);

            public:
            /// Checks whether or not the device has internet.
            /// NOTE: This method is blocking.
            /// Calling this method is thread-safe.
            void internet_connection_test();
            bool is_connected() const;
            void disconnect();
            bool has_internet() const;
            //decltype(ws)& get_websocket();
            /// Gets this device's network internal IP address.
            Option<std::string> ip();
            Option<std::string> ips();
            /// Scans for nearby wireless networks.
            /// NOTE: This method is blocking.
            /// Calling this method is thread-safe.
            std::vector<WifiNetworkInfo> scan();
            /// Connects to a given wireless network.
            /// Also checks for network connectivity.
            /// NOTE: This method is blocking.
            /// Calling this method is thread-safe.
            WifiConnectionResult connect(WifiNetworkInfo const& network, std::string psk);
            //friend void poll_websocket(lv_timer_t* timer);
        };

        
        
        
    }

    namespace admin_app{

        struct AdminInfo {
            std::string ip;
            std::string port;
            std::string name;
            Option<easywsclient::WebSocket::pointer> socket;
        };
        
        class AdminState {
            wifi::WifiState& ws;
            Option<AdminInfo> current_admin;
            std::mutex connecting_mutex, scan_mutex;

            public:
            AdminState(wifi::WifiState& ws);
            /// Attempts to connect to an admin app
            /// Returns a websocket on success.
            /// Sets current_admin on success.
            /// NOTE: This method is blocking.
            /// Calling this method is thread-safe
            Option<easywsclient::WebSocket::pointer> connect_to_admin_app(AdminInfo const& admin);
            /// Scans for administrators on the device's network.
            /// NOTE: This method is blocking.
            /// Calling this method is thread-safe
            std::vector<AdminInfo> scan(std::string const& port);
            bool is_connecting();
            bool is_connected() const;
            void send_data(std::string const& data) const;

            friend void poll_admin_app(AdminState* state);
            friend void poll_websocket(AdminState* state);
        };
        
        void poll_admin_app(AdminState* state);
        void poll_websocket(AdminState* state);
    }

    namespace json {

        using nlohmann::json;
        using nlohmann::json_schema::json_validator;

        const std::string _ph = "PLACEHOLDER";

        const json ssgcData = {
            {"ssgcType", "clientData"},
            {"clientIP", _ph},
            {"clientName", _ph},
            {"data", _ph}
        };

        bool validate(json const& obj, json const& schema);
        
    }

    class State{
        std::mutex screenshot_mutex;

        public:
        wifi::WifiState ws;
        admin_app::AdminState as;
        State();
        /// Takes a screenshot and saves it in ./image.bin
        /// NOTE: This method is blocking
        ///
        void take_screenshot();
        void screenshot_handle();
        void connect_to_admin_app();
        
        friend void screenshot_cb(State* state);
    };

    void screenshot_cb(State* state);
}

extern calc_state::State global_state;

#endif