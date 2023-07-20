#ifndef LV_GRAPH
#define LV_GRAPH

#include <lv_drv_conf.h>
#include <lvgl/lvgl.h>
#include <iostream>
#include <sstream>
#include <string>
#include <functional>
#include <array>
#include <math.h>
#include <limits> 
#include <vector>
#include <chrono>
#include <option.hxx>


#if ENABLE_GIAC == 1
#if ENABLE_LINUX
#include <giac/config.h>
#endif
#include <giac/gen.h>
#include <giac/giac.h>
#endif


#define MONITOR_HOR_RES 320
#define MONITOR_VER_RES 240
#define ENABLE_EXPERIMENTAL_PLOTTING 1
#if ENABLE_EXPERIMENTAL_PLOTTING
#endif

namespace graphing {

    #define COLOR(r,g,b) (lv_color_t)LV_COLOR_MAKE(r,g,b)
    
    using graph_function = std::function<double(double)>;
    
    inline double calculate_hyp(double a, double b) {
        return sqrt(a*a + b*b);
    }

    void create_graph(lv_obj_t *parent);

    struct Point{
        double x, y;

        Point() = default;

        constexpr Point(double x, double y):x(x),y(y){}

        Point(lv_point_t point):x(point.x),y(point.y){}

        std::string to_string() const;

        /// Unsafely truncate an lv_point_t from a Point.
        inline lv_point_t to_lv_point() const {
            lv_point_t point{(lv_coord_t)x, (lv_coord_t)y};
            return point;
        }

    };

    /// Used for cache when plotting functions.
    struct PlotDataBlock{
        using DataBlock = std::vector<Point>;
        double x_min, x_max; // holds the min and max x coords of the vector of points.
        DataBlock data;

        PlotDataBlock() = default;
        PlotDataBlock(double x_min, double x_max, DataBlock data):
        x_min(x_min), x_max(x_max), data(std::move(data)){}
    };

    enum PlotRecalculationType {
        RecalculateNothing,
        RecalculateLeft,
        RecalculateRight,
        RecalculateAll,
    };

    struct Plot{
        using DataBlock = std::vector<Point>;
        using DataRange = std::pair<double, double>;
        lv_draw_line_dsc_t style;
        std::string name; // Ex: f1, f2, f3, f4, etc.
        std::string function_expression; // Ex: sin(x), x^2, 4x+2, etc.
        std::vector<PlotDataBlock> cached_data;
        double cached_scale; // used to check if the scale is the same
        bool last_drew; // used to keep track if the last function assigned was able to draw onto the screen.
        double block_width; // set to half of the viewport's virtual width.
        #if ENABLE_GIAC
        giac::context& ctx; // context from the graph
        #endif
        
        /// Create a Plot that holds the name, expression, and color of a function being displayed on the graph.
        #if ENABLE_GIAC
        Plot(std::string name, std::string function_expression, lv_color_t color, double viewport_virtual_width, giac::context& ctx):
        name(std::move(name)), function_expression(std::move(function_expression)), block_width(viewport_virtual_width/2.0), ctx(ctx)
        #else 
        Plot(std::string name, std::string function_expression, lv_color_t color, double viewport_virtual_width):
        name(std::move(name)), function_expression(std::move(function_expression)), block_width(viewport_virtual_width/2.0)
        #endif
        {
            lv_draw_line_dsc_init(&style);
            style.color = color;
            cached_data.resize(6, PlotDataBlock{0.0, 0.0, std::vector<Point>()}); // six DataBlocks per Plot. Two for the viewport, and two on each side of the viewport.
            last_drew = false; // first time drawing.
        } 

        #if ENABLE_GIAC
        template<typename T>
        T giac_call(T data) const {
            static char fcall_buf[128] = {0};
            std::sprintf(fcall_buf, "%s(", name.c_str());
            if (std::is_same<T, double>::value){
                sprintf(fcall_buf+name.size()+1, "%.7lf)", data);
                giac::gen g(fcall_buf, &ctx);
                return (T)std::stod(giac::gen2string(giac::eval(g, &ctx)));
            } else {
                throw std::runtime_error("Unsupported type in giac_call.");
            }
        }
        #endif
        /// Change the expression of the function, and set flag to enable redrawing.
        void change_expression(std::string str){
            function_expression = std::move(str);
            last_drew = false;
        }

        /// Returns plot calculation for a given range.
        std::pair<std::vector<double>, std::vector<double>> calculate(DataRange const& data_range);

        /// Possibly calculate data given a certain range, scale, and center_x coordinate (of the viewport's virtual center).
        /// If nothing needs to be calculated, no work will be done.
        void maybe_calculate(DataRange const& data_range, double scale, double center_x){
            PlotRecalculationType rtype = needs_recalculation(data_range, scale);
            if (rtype == RecalculateNothing) // if no recalculation is needed, then do nothing and just return.
                return;
            cached_scale = scale;
            DataRange x_range = get_calculation_range(rtype, center_x);
            decltype(calculate(x_range)) calculation;
            try {
                calculation = calculate(x_range); // throws
                last_drew = true;
            } catch (...) {
                last_drew = false;
                throw 21; // doesn't really mean much, it just has to throw.
            }

            if (rtype == RecalculateLeft){
                for (std::size_t i = 0; i < calculation.first.size(); i++){
                    cached_data.at(0).data.push_back(Point(std::move(calculation.first[i]), std::move(calculation.second[i])));
                }
            } else if (rtype == RecalculateRight){
                for (std::size_t i = 0; i < calculation.first.size(); i++){
                    cached_data.at(5).data.push_back(Point(std::move(calculation.first[i]), std::move(calculation.second[i])));
                }
            } else { // Recalculate All
                cached_data.clear();
                for (std::size_t i = 0, j = 0; i < 6; i++){
                    cached_data.push_back(PlotDataBlock(x_range.first+block_width*((double)i), x_range.first+block_width*((double)i+1.0), DataBlock()));
                    for (;j < calculation.first.size() && cached_data.at(i).x_max > calculation.first[j]; j++){
                        cached_data.at(i).data.push_back(Point(std::move(calculation.first[j]), std::move(calculation.second[j])));
                    }
                }
            }
        }

        private:

        /// Returns whether or not the plot needs to recalculate data depending on the range given in `data_range`
        PlotRecalculationType needs_recalculation(DataRange const& data_range, double scale){
            if (cached_data.at(1).x_min >= data_range.second || cached_data.at(4).x_max <= data_range.first || scale != cached_scale || !last_drew)
                return RecalculateAll;
            if (cached_data.at(1).x_min >= data_range.first)
                return RecalculateLeft; 
            if (cached_data.at(4).x_max <= data_range.second)
                return RecalculateRight;
            return RecalculateNothing;
        }

        /// Returns a range that will need to be calculated given a PlotReclculationType and a center x_coord in the 
        /// case that the entire plot needs to be recalculated (which will be centered around the given coord).
        /// SIDE EFFECTS: If recalculating, this function will shift/remove values within cached_data
        DataRange get_calculation_range(PlotRecalculationType prt, double center_x) {
            switch (prt){
                case RecalculateNothing:
                    return DataRange(0.0, 0.0); // returns (0,0) in the case that nothing needs to be recalculated.
                case RecalculateLeft:
                    // shift values right. Rightmost block gets replaced. Leftmost block gets recalculated
                    for (ssize_t i = ((ssize_t)cached_data.size())-1; i > 0; i--){ 
                        cached_data.at(i) = std::move(cached_data.at(i-1));
                    }
                    cached_data.at(0) = PlotDataBlock(cached_data.at(1).x_min - block_width, cached_data.at(1).x_min, DataBlock());
                    return DataRange(cached_data.at(0).x_min, cached_data.at(0).x_max);
                case RecalculateRight:
                    // shift values left. Leftmost block gets replaced. Rightmost block gets recalculated.
                    for (ssize_t i = 0; i < ((ssize_t)cached_data.size())-1; i++){
                        cached_data.at(i) = std::move(cached_data.at(i+1));
                    }
                    cached_data.at(5) = PlotDataBlock(cached_data.at(4).x_max, cached_data.at(4).x_max + block_width, DataBlock());
                    return DataRange(cached_data.at(5).x_min, cached_data.at(5).x_max);
                case RecalculateAll:
                    return DataRange(center_x - block_width*3.0, center_x + block_width*3.0);
                default:
                    return DataRange(0.0, 0.0); // Should never happen, but will return (0.0, 0.0) just in case.
            }
        }
    };

    class Graph{
        // Change this to fit your needs.
        static const int GRAPH_BUF_SIZE = LV_CANVAS_BUF_SIZE_TRUE_COLOR(MONITOR_HOR_RES, MONITOR_VER_RES - 20);
        static const int VIEWPORT_WIDTH = MONITOR_HOR_RES;
        static const int VIEWPORT_HEIGHT = MONITOR_VER_RES - 20;
        static const int DEFAULT_LINE_WIDTH = 1;
        static const int BOLD_LINE_WIDTH = 2;
        const std::array<lv_color_t, 7> default_colors{  
            COLOR(255, 0, 0), 
            COLOR(0, 255, 0),
            COLOR(0, 0, 255),
            COLOR(0, 0, 255),
            COLOR(255, 255, 0),
            COLOR(255, 0, 255),
            COLOR(0, 255, 255)};
        
        double VIEWPORT_HYP;

        #if ENABLE_GIAC == 1
        giac::context ctx;
        #endif
        
        Point offset;
        double scale;

        lv_color_t buf[GRAPH_BUF_SIZE];
        
        const char* function_button_map[3] = {"+", "-", ""}; // text used for the function buttons.
        const std::string options_button_text = "this\nis\na\ntest";
        lv_obj_t* canvas, *function_button, *function_text_area, *zoom_buttons, *options_button;
        lv_draw_line_dsc_t axes_style;
        
        std::vector<Plot> plot_list;
        int current_function_number = 1; // defaults to 1
    public:
        Graph(lv_obj_t* parent);
        //Graph(lv_obj_t* parent, lv_obj_t* function_button);

        void translate_center(Point vec){
            offset.x += vec.x*scale;
            offset.y += vec.y*scale;
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

        inline double horizontal_scale() const {
            return scale*cos(hyp_angle());
        }

        inline double vertical_scale() const {
            return scale*sin(hyp_angle());
        }

        inline double real_width() const {
            return VIEWPORT_HYP*horizontal_scale();
        }

        inline double real_height() const {
            return VIEWPORT_HYP*vertical_scale();
        }

        inline double get_scale() const {
            return scale;
        }

        inline void set_scale(double s){
            translate_center(Point{offset.x*(s - scale), offset.y*(s - scale)});
            scale = s;
        }

        inline void scale_delta(double s){
            set_scale(get_scale()+s);
        }

        lv_obj_t* get_canvas() const {
            return canvas;
        }

        template<typename T>
        inline double virtual_to_viewport_x(T const& x) const {
            return (x+offset.x)/scale;
        }

        template<typename T>
        inline double virtual_to_viewport_y(T const& y) const {
            return (-y+offset.y)/scale + VIEWPORT_HEIGHT;
        }
        
        template<typename T>
        inline double viewport_to_virtual_x(T const& x) const {
            return x*scale - offset.x;
        }

        template<typename T>
        inline double viewport_to_virtual_y(T const& y) const {
            return -(y-VIEWPORT_HEIGHT)*scale + offset.y;
        }      

        Point bottom_left_real() const;
        Point bottom_right_real() const;
        Point top_left_real() const;
        Point top_right_real() const; 

        Point virtual_to_viewport(double x, double y) const;
        Point viewport_to_virtual(double x, double y) const;

        std::pair<double, double> viewport_virtual_domain() const;
        std::pair<double, double> viewport_virtual_range() const;

        //void draw_function(graph_function);
        //void draw_function(Plot const&);
        //void draw_function(graph_function, lv_color_t);
        void draw_function(int id, lv_color_t);
        
        std::string next_function_name();
        Plot* get_plot(std::string const&);
        Plot* get_plot(int id);
        void add_function(std::string func);
        /// Update function by entering function definition as would be written in giac.
        /// EX: 'f(x):=4*x^2'
        void update_function(std::string func_def);
        void update();

        void set_function_button(lv_obj_t* button);
        std::string get_function_button_selected_str() const;
        int get_function_button_selected_id() const;
        void set_function_textarea(lv_obj_t* textarea);
        void set_function_textarea_str(std::string const&);
        std::string get_plots_fmt_str() const;
        int get_newest_function_int() const;
        std::string get_newest_function_str() const;
        std::string get_newest_function_id_str() const;
        // Change the currently bolded function to a different one.
        void switch_to_bold(int id);

        // function color settings

    private:
        #if ENABLE_GIAC
        template<typename T>
        T giac_call(std::string& func_name, T data) const;
        #endif
        void draw_axes();
        /// Draws both the ticks and the text for the tick values.
        void draw_ticks();
        void fill_background();
        // Cycles between colors in default_colors array.
        lv_color_t get_next_color() const;

    };
    // blr.y - (blr.y - y) = blr.y - blry + y = y
}

#endif
