#ifndef LV_GRAPH
#define LV_GRAPH

#include "../../../lv_drv_conf.h"
#include "../../../lvgl/lvgl.h"
#include <iostream>
#include <sstream>
#include <gmpxx.h>
#include <string>
#include <functional>
#include <array>
#include <math.h>
#include <limits> 
#include <vector>
#include <chrono>

namespace graphing {

    constexpr int MPF_PRECISION = 200;

    #define CREATE_MPF(x) mpf_class(x, MPF_PRECISION)
    
    using graph_function = std::function<mpf_class(mpf_class)>;
    
    inline double calculate_hyp(double a, double b) {
        return sqrt(a*a + b*b);
    }

    void create_graph(void);

    struct Point{
        mpf_class x, y;

        Point() = default;

        Point(std::string const& x, std::string const& y){
            this->x = mpf_class(x, MPF_PRECISION);
            this->y = mpf_class(y, MPF_PRECISION);
        }

        Point(mpf_class x, mpf_class y):x(x),y(y){}

        Point(lv_point_t point):x(point.x),y(point.y){}

        std::string to_string() const {
            std::stringstream str;
            str << "(" << x.get_d() << " " << y.get_d() << ")";
            return str.str();
        }

        inline lv_point_t to_lv_point() const {
            lv_point_t point{(lv_coord_t)x.get_si(), (lv_coord_t)y.get_si()};
            return point;
        }

    };

    struct Plot{
        graph_function func;
        lv_draw_line_dsc_t style;

        Plot(graph_function func, lv_color_t color):func(func){
            lv_draw_line_dsc_init(&style);
            style.color = color;
        }
    };

    class Graph{
        // Change this to fit your needs.
        static const int GRAPH_BUF_SIZE = LV_CANVAS_BUF_SIZE_TRUE_COLOR(MONITOR_HOR_RES, MONITOR_VER_RES);
        static const int VIEWPORT_WIDTH = MONITOR_HOR_RES;
        static const int VIEWPORT_HEIGHT = MONITOR_VER_RES;
        double VIEWPORT_HYP;
        
        Point offset;
        mpf_class scale;

        lv_color_t buf[GRAPH_BUF_SIZE];

        lv_obj_t* canvas;
        lv_draw_line_dsc_t axes_style;
        
        std::vector<Plot> plot_list;
    public:

        Graph(lv_obj_t* parent);

        void translate_center(Point vec){
            offset.x += vec.x;
            offset.y += vec.y;
            update();
        }

        template<typename T>
        void translate_center(T x, T y){
            offset.x += x;
            offset.y += y;
            update();
        }
        // a^2 + b^2 = c^2
        // c = hyp*scale
        // a = cos(theta)*hyp*scale
        // b = sin(theta)*hyp*scale

        /// returns the angle of the viewport from corner-to-corner in radians.
        inline double hyp_angle() const {
            return atan2(VIEWPORT_HEIGHT, VIEWPORT_WIDTH);
        }

        inline mpf_class horizontal_scale() const {
            return scale*cos(hyp_angle());
        }

        inline mpf_class vertical_scale() const {
            return scale*sin(hyp_angle());
        }

        inline mpf_class real_width() const {
            return VIEWPORT_HYP*horizontal_scale();
        }

        inline mpf_class real_height() const {
            return VIEWPORT_HYP*vertical_scale();
        }

        inline void set_scale(mpf_class s){
            scale = s;
        }      

        Point bottom_left_real() const;
        Point bottom_right_real() const;
        Point top_left_real() const;
        Point top_right_real() const; 

        Point real_to_viewport(mpf_class x, mpf_class y) const;
        Point viewport_to_real(mpf_class x, mpf_class y) const;

        std::pair<mpf_class, mpf_class> viewport_real_domain() const;
        std::pair<mpf_class, mpf_class> viewport_real_range() const;

        void draw_function(graph_function);
        void draw_function(Plot const&);
        void draw_function(graph_function, lv_color_t);
        
        void add_function(graph_function);
        void add_function(Plot const&);
        void add_function(graph_function, lv_color_t);
        void update();
    private:

        void draw_axes();
        void fill_background();
        

    };
    // blr.y - (blr.y - y) = blr.y - blry + y = y
}

#endif