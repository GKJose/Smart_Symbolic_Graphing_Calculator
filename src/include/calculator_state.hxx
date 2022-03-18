#ifndef CALCULATOR_STATE_HEADER
#define CALCULATOR_STATE_HEADER

#include <async_extensions.hxx>
#include <option.hxx>
#include <lv_drv_conf.h>
#include <lvgl/lvgl.h>

namespace state {

    // struct GraphingInfo{
    //     double time_to_plot_ms;
    //     double longset_time_to_plot_ms;
    //     double fastest_time_to_plot_ms;
    // };

    namespace updating{

        using OptHandle = Option<std::future<std::string>>;

        enum UpdateState{
            Idle,
            Downloading,
            InstallConfirmation,
            Cleaning,
            Making,
            Done,
            RestartConfirmation,
            Restart
        };

        struct UpdateInfo{
            UpdateState state;
            OptHandle update_download_handle;
            OptHandle update_clean_handle;
            OptHandle update_make_handle;
        };

        void update_daemon_cb(lv_timer_t* timer);

        void reset_update_info();
        UpdateInfo& get_update_info();
        UpdateState get_update_state();
        void advance_update_state();
        
    };

    
    

};


#endif