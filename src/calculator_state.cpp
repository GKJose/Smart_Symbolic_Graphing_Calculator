#include <calculator_state.hxx>
#include <chrono>
#include <regex>
#include <iostream>

using namespace state::updating;
using namespace std::chrono_literals;
#define _SU state::updating
#define GUARD_READY(x) {\
    if (x.value_ref().wait_for(0ms) != std::future_status::ready) \
        return; \
    }

#define MSGBOX(mbox, title, text, add_close_btn) {\
    mbox = lv_msgbox_create(nullptr, title, text, nullptr, add_close_btn);\
    lv_obj_center(mbox);\
}
#define MSGBOX_YN(mbox, title, text, add_close_btn) {\
    mbox = lv_msgbox_create(nullptr, title, text, button_yes_no_txt, add_close_btn);\
    lv_obj_center(mbox);\
    lv_obj_add_event_cb(mbox, install_button_cb, LV_EVENT_VALUE_CHANGED, nullptr);\
}

static void install_button_cb(lv_event_t* event){
    lv_obj_t* button_matrix = lv_event_get_target(event);
    uint16_t button = lv_btnmatrix_get_selected_btn(button_matrix);
    if (button == 0) {
        advance_update_state(); // go to clean state
    } else if (button == 1){
        lv_msgbox_close(lv_event_get_current_target(event));
        reset_update_info();
    };
}

void _SU::update_daemon_cb(lv_timer_t* timer){
    static lv_obj_t* mbox_download = nullptr, 
        *mbox_download_confirm = nullptr, 
        *mbox_clean = nullptr, 
        *mbox_make = nullptr, 
        *mbox_restart_confirm = nullptr;
    static const char* button_yes_no_txt[] = {"YES", "NO", ""}; 
    UpdateInfo& info = get_update_info();

    if (info.state == Idle)
        return;

    if (info.state == Downloading){
        if (!info.update_download_handle){
            info.update_download_handle = run_async("git pull origin master");
            MSGBOX(mbox_download, "Downloading Update", "The system is downloading the latest version from SSGC's master branch.", false)
        }

        GUARD_READY(info.update_download_handle)

        std::string value = info.update_download_handle.value_ref().get();
        if(std::regex_search(value, std::regex("Already up to date"))){
            lv_msgbox_close(mbox_download); // when a messagebox is closed the memory is freed.
            MSGBOX_YN(mbox_download_confirm,
                "Download Finished", 
                "The currently installed version is already up to date. No installation required. Install anyway?", false)
            advance_update_state();
            return;
        } else if (std::regex_search(value, std::regex("Updating"))){
            lv_msgbox_close(mbox_download);
            MSGBOX_YN(mbox_download_confirm,
                "Download Finished",
                "The most recent version os SSGC has been sucessfully downloaded. Install?", false)
            advance_update_state();
            return;
        } else {
            lv_msgbox_close(mbox_download);
            MSGBOX(mbox_download_confirm, "ERROR", "Unable to fetch the latest version of SSGC from the repo. Try again later.", true)
            reset_update_info();
            return;
        }
    }

    if (info.state == InstallConfirmation) return; // Wait for the user to want to install latest version.

    if (info.state == Cleaning){
        if (!info.update_clean_handle){
            info.update_clean_handle = run_async("cd src && make clean && cd ..");
            MSGBOX(mbox_clean, "Removing old files", "Cleaning out the old files that are currently installed.", false);
        }

        GUARD_READY(info.update_clean_handle)

        advance_update_state(); // cleaning has finished.
        lv_msgbox_close(mbox_clean);
        return;
    }

    if (info.state == Making){
        if (!info.update_make_handle){
            info.update_make_handle = run_async("make SSGC -j");
            MSGBOX(mbox_make,
                "Building", 
                "Building the new, updated version of SSGC from source. Please wait until the build has finished.", false);
        }

        GUARD_READY(info.update_make_handle)

        advance_update_state(); // done building
        lv_msgbox_close(mbox_make);
        return;
    }

    if (info.state == Done){
        MSGBOX_YN(mbox_restart_confirm,
        "Update Successful.", 
        "Congratulations, you have successfully updated to the latest version of SSGC!. Do you wish restart the application now?", false);
        advance_update_state();
        return;
    }

    if (info.state == RestartConfirmation) return; // wait for the user to decide on whether or not to restart the app.
    lv_msgbox_close(mbox_restart_confirm);
    if (info.state == Restart){
        auto h1 = run_async("pwd");
        std::cout << h1.get();
        reset_update_info();
        //auto handle = run_async("./build/out/SSGC"); // start a new detached process
        //std::exit(EXIT_SUCCESS); // end the programmo
    }
}

void _SU::reset_update_info(){
    UpdateInfo& info = get_update_info();
    info.state = Idle;
    info.update_download_handle = OptNone;
    info.update_clean_handle = OptNone;
    info.update_make_handle = OptNone;
}
UpdateInfo& _SU::get_update_info(){
    static UpdateInfo info = {Idle, OptNone, OptNone, OptNone};
    UpdateInfo& info_ref = info;
    return info_ref;
}
UpdateState _SU::get_update_state(){
    return get_update_info().state;
}
void _SU::advance_update_state(){
    UpdateInfo& info = get_update_info();
    if (info.state != Done){
        info.state = (UpdateState)((int)info.state + 1);
    }
}