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


#define UNDEFINED_CODE_BREAK 1

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
            std::mutex connecting_mutex, disconnecting_mutex, scan_mutex, permission_mutex, info_mutex, reply_mutex;
            Option<AdminInfo> current_admin;
            std::string _device_name = "UNKNOWN";
            public:
            AdminState(wifi::WifiState& ws);
            /// Attempts to connect to an admin app
            /// Returns a websocket on success.
            /// Sets current_admin on success.
            /// NOTE: This method is blocking.
            /// Calling this method is thread-safe
            #if UNDEFINED_CODE_BREAK
            void connect(AdminInfo const& admin);
            #else
            Option<easywsclient::WebSocket::pointer> connect(AdminInfo& admin);
            #endif

            /// Disconnects from an admin app.
            /// NOTE: This method is blocking
            /// Calling this method is thread-safe.
            void disconnect();
            /// Scans for administrators on the device's network.
            /// NOTE: This method is blocking.
            /// Calling this method is thread-safe.
            std::vector<AdminInfo> scan(std::string const& port = "6969");
            bool is_connecting();
            bool is_connected() const;
            /// Attempts to get permissions from the current admin.
            /// Sends a JSON object that complies with the connectionPermission schema
            /// Receives a connectionPermissionReply JSON object from an admin and returns the
            /// permissions as a vector of strings.
            /// NOTE: This method is blocking
            /// Calling this method is thread-safe.
            Option<std::vector<std::string>> get_permissions();
            /// Attempts to get information about an admin.
            /// Sends a JSON object that complies with the connectionRequest schema
            /// Receives a adminInfo JSON object from an admin and returns the information
            /// as an AdminInfo struct.
            /// NOTE: This method is blocking
            /// Calling this method is thread-safe.
            Option<AdminInfo> get_admin_info(std::string const& ip);
            Option<AdminInfo> get_current_admin();
            void send_data(std::string const& data);
            void set_device_name(std::string new_name);
            std::string device_name() const;

            friend void poll_admin_app(AdminState* state);
            friend void poll_websocket(AdminState* state);
        };
        
        void poll_admin_app(AdminState* state);
        void poll_websocket(AdminState* state);
    }

    namespace json {

        using nlohmann::json;
        using nlohmann::ordered_json;
        using nlohmann::json_schema::json_validator;

        const std::string _ph = "PLACEHOLDER";

        const ordered_json ssgcData = {
            {"ssgcType", "clientData"},
            {"clientIP", _ph},
            {"clientName", _ph},
            {"data", _ph}
        };
        const ordered_json connectionInfo = {
            {"ssgcType", "connectionInfo"},
            {"clientIP", _ph},
            {"clientName", _ph},
            {"clientVersion", {
                {"major",0},
                {"minor",0},
                {"bugfix",0}
            }}
        };
        const ordered_json connectionRequest = {
            {"ssgcType", "connectionRequest"},
            {"clientIP", _ph},
            {"clientName", _ph},
            {"clientVersion", {
                {"major",0},
                {"minor",0},
                {"bugfix",0}
            }}
        };
        const ordered_json connectionRevoke = {
            {"ssgcType", "clientRevoke"},
            {"revokeReason","clientDisconnection"},
            {"clientIP", _ph},
            {"clientName", _ph}
        }; 
        const ordered_json permissionAcceptReply = {
            {"ssgcType","connectionPermissionAccept"},
            {"clientIP", _ph},
            {"clientName", _ph}
        };
        const ordered_json permissionRejectReply = {
            {"ssgcType","connectionPermissionReject"},
            {"clientIP", _ph},
            {"clientName", _ph}
        };
        bool validate(json const& obj, json const& schema);
        
    }

    /// Holds general state of the application.
    class State{
        std::mutex screenshot_mutex;
        lv_timer_t* screenshotTimer;
        lv_timer_t* pollWebsocketTimer;
        public:
        wifi::WifiState ws;
        admin_app::AdminState as;
        nlohmann::ordered_json permissions;
        std::string port;
        State();
        void set_screenshot_timer();
        void set_websocket_timer();
        /// Takes a screenshot and saves it in ./image.bin
        /// NOTE: This method is blocking
        ///
        void take_screenshot();
        void screenshot_handle();
        bool connect_to_admin_app(admin_app::AdminInfo& admin);
        
        friend void screenshot_cb(State* state);
    };

    void screenshot_cb(State* state);
}

extern calc_state::State global_state;

#endif