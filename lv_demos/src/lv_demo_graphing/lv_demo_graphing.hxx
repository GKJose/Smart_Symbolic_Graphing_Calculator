#ifndef LV_DEMO_GRAPHING
#define LV_DEMO_GRAPHING

// #ifdef __cplusplus
// extern "C" {
// #endif

#include "../../../lv_drv_conf.h"
#include "../../../lvgl/lvgl.h"
// #include "../../lv_demo.h"

// #ifdef __cplusplus
// }
// #endif

#include <iostream>
#include <sstream>
#include <gmpxx.h>



namespace lv_demo_graphing {

    constexpr int MPF_PRECISION = 200;

    #define CREATE_MPF(x) mpf_class(x, MPF_PRECISION)

    void create_graph(void);

    struct Point{
        mpf_class x, y;

        Point() = default;

        Point(std::string const& x, std::string const& y){
            this->x = mpf_class(x, MPF_PRECISION);
            this->y = mpf_class(y, MPF_PRECISION);
        }

        Point(mpf_class x, mpf_class y):x(x),y(y){}

        std::string to_string() const {
            std::stringstream str;
            str << "(" << x << " " << y << ")";
            return str.str();
        }

    };

    class Graph{
        // Change this to fit your needs.
        static const int GRAPH_BUF_SIZE = LV_CANVAS_BUF_SIZE_TRUE_COLOR(MONITOR_HOR_RES, MONITOR_VER_RES);
        static const int VIEWPORT_WIDTH = MONITOR_HOR_RES;
        static const int VIEWPORT_HEIGHT = MONITOR_VER_RES;
        
        Point center;
        mpf_class scale;

        lv_color_t buf[GRAPH_BUF_SIZE];
        bool diffbuf[VIEWPORT_WIDTH * VIEWPORT_HEIGHT];

        lv_obj_t* canvas;
        lv_draw_line_dsc_t axes_style;
    public:

        Graph(lv_obj_t* parent);

        inline mpf_class real_width() const {
            return VIEWPORT_WIDTH*scale;
        }

        inline mpf_class real_height() const {
            return VIEWPORT_HEIGHT*scale;
        }

        Point bottom_left_real() const;
        Point bottom_right_real() const;
        Point top_left_real() const;
        Point top_right_real() const; 

        Point real_to_viewport(mpf_class x, mpf_class y) const;
        Point viewport_to_real(mpf_class x, mpf_class y) const;

        std::pair<mpf_class, mpf_class> viewport_real_domain() const;
        std::pair<mpf_class, mpf_class> viewport_real_range() const;
    private:

        int oof = 4;

        void draw_axes();

    };
    // blr.y - (blr.y - y) = blr.y - blry + y = y
}

#endif