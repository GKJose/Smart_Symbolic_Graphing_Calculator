#include <graph.hxx>
#include <algorithm_extensions.hxx>
#include <option.hxx>
#include <ticks.hxx>
#include <random>
#include <type_traits>
#include <stack>
#include <Calculator.h>
#include <state.hxx>
// #if ENABLE_EXPERIMENTAL_PLOTTING
// #include <armadillo>
// #endif

namespace graphing {

    /// obj is an lv_canvas
    static void graph_event_cb(lv_event_t* event){
        static lv_point_t last_point{0,0};
        static auto last_time = lv_tick_get();

        if (lv_tick_elaps(last_time) > 100) {
            last_point = lv_point_t{0,0};
        }
        static lv_point_t point;
        lv_indev_t* indev = lv_indev_get_act();
        lv_indev_get_point(indev, &point);
        Graph* graph = static_cast<Graph*>(event->user_data);;
        //std::cout << "POINT: (" << point.x << " " << point.y << ")\n";
        if (last_point.x != 0 && last_point.y != 0){
            lv_point_t delta{(lv_coord_t)(point.x - last_point.x), (lv_coord_t)(point.y - last_point.y)};
            graph->translate_center(delta);
            graph->update();
        }
        last_point = point;
        last_time = lv_tick_get();
    }

    static void zoom_btnmatrix_cb(lv_event_t* event){
        uint32_t id = lv_btnmatrix_get_selected_btn(event->target);
        auto graph = static_cast<graphing::Graph*>(event->user_data);
        if (id == 0) {
            graph->set_scale(graph->get_scale()*0.9);
        } else {
            graph->set_scale(graph->get_scale()*1.1);
        }
        graph->update();
    }

    static void textarea_cb(lv_event_t* event){
        auto graph = static_cast<graphing::Graph*>(event->user_data);
        auto func_str = graph->get_function_button_selected_str();
        auto func_id = graph->get_function_button_selected_id();
        auto func_expression = std::string(lv_textarea_get_text(event->target));
        auto func_textArea = (lv_obj_t*) event->target;
        auto plot = graph->get_plot(func_id);
        std::cout << "TEXTAREA_CB: " << func_str << " " << func_expression << " " << func_id << "\n";
        if(global_state.permissions != nullptr && global_state.permissions["permissions"]["graphingRestrictionsEnable"].get<bool>()){
			auto found = std::string::npos;
			
			std::vector<std::string> blacklistedFunctions = (global_state.permissions["graphingInfo"]["graphingBlacklist"] != nlohmann::detail::value_t::null) ? global_state.permissions["graphingInfo"]["graphingBlacklist"].get<std::vector<std::string>>() : std::vector<std::string>();
			for(std::string function: blacklistedFunctions){
				found = func_expression.find(function);
				if(found != std::string::npos){
					break;
				}
			}
			if(found == std::string::npos){
				graph->update_function(func_str + "(x):=" + func_expression);
                plot->name = std::move(func_str);
                plot->change_expression(std::move(func_expression));
                graph->update(); 

			}else{
                auto msgbox = lv_msgbox_create(NULL,"Blacklisted Function","The function is blacklisted!",nullptr,true);
                lv_obj_center(msgbox);
				lv_textarea_set_text(func_textArea,"");
			}
		}else{
            graph->update_function(func_str + "(x):=" + func_expression);
            plot->name = std::move(func_str);
            plot->change_expression(std::move(func_expression));
            graph->update();
		}
          
    }

    static void dropdown_button_cb(lv_event_t* event){
        char buf[10];
        std::cout << "DROPDOWN_BUTTON_CB\n";
        int id = lv_dropdown_get_selected(event->target);
        lv_dropdown_get_selected_str(event->target, buf, sizeof(buf)); // C function, so no std::string
        auto graph = static_cast<graphing::Graph*>(event->user_data);

        // Check if user selected to add a function to the graph.
        if (buf[0]=='+'){
            graph->add_function("");
            lv_dropdown_set_selected(event->target, id);
            graph->set_function_textarea_str("");
            return;
        }
        graph->switch_to_bold(id);
        // Finds user-selected plot.
        graphing::Plot* plot = graph->get_plot(id);
        if (plot == nullptr)
            graph->set_function_textarea_str("NULL");
        else {
            graph->set_function_textarea_str(plot->function_expression);
        } 
        graph->update();
    }

    std::pair<std::vector<double>, std::vector<double>> Plot::calculate(DataRange const& data_range){
        #if ENABLE_GIAC
            // Sets the plots to be drawn by points, rather than lines.
            #define ENABLE_DBGPLOT 0
            #define ENABLE_DBGPRINT 0
            #define GEN_RAND() ((double)(rand())/((double)RAND_MAX))
            #if ENABLE_DBGPRINT
            #define DBGPRINT(x) std::cout << #x": " << x << "\n"
            #else 
            #define DBGPRINT(x)
            #endif
            
            
            // Code from here is translated from Julia to a C++ equivalent. 
            // https://github.com/JuliaPlots/PlotUtils.jl/blob/master/src/adapted_grid.jl

            const std::size_t max_recursions = 7;
            const double max_curvature = 0.05; // was 2

            std::size_t n_points = 21;
            std::size_t n_intervals = n_points / 2;
            assert(n_points % 2 == 1); // assert that n_points is odd

            std::vector<double> xs = algext::linspace(data_range.first, data_range.second, n_points);
            // Move the first and last interior points a bit closer to the end points
            xs[1] = xs[0] + (xs[1] - xs[0]) * 0.25;
            xs[xs.size()-2] = xs[xs.size()-1] - (xs[xs.size()-1] - xs[xs.size()-2]) * 0.25;

            // Wiggle interior points a bit to prevent aliasing and other degenerate cases
            std::mt19937 rng(1337);
            std::uniform_real_distribution<double> _rand(0.0, 1.0);
            double rand_factor = 0.05;
            
            for (std::size_t i = 1; i < xs.size() - 2; i++){
                xs[i] += rand_factor * 2 * (_rand(rng) - 0.5) * (xs[i+1] - xs[i-1]);
            }

            std::vector<std::size_t> n_tot_refinements(n_intervals, 0);
            
            // Evaluate the function on the whole interval
            std::vector<double> fs(xs.size());
            for (std::size_t i = 0; i < xs.size(); i++){
                fs[i] = giac_call(xs[i]); // will throw if something's bad.
            }
            
            while (true){
                std::vector<double> curvatures(n_intervals, 0.0);
                std::vector<bool> active(n_intervals, false);
                using dlimits = std::numeric_limits<double>;
                double min_f = dlimits::infinity(), max_f = -dlimits::infinity();
                for (std::size_t i = 0; i < fs.size(); i++){
                    if (!std::isnan(fs[i])){
                        if (fs[i] < min_f)
                            min_f = fs[i];
                        else if (fs[i] > max_f)
                            max_f = fs[i];
                    }
                }
                // If all values were NaN, then set min and max to 0.0
                if (min_f == dlimits::infinity() && max_f == -dlimits::infinity()){
                    min_f = 0.0; max_f = 0.0;
                }
                double f_range = max_f - min_f;
                // Guard against division by zero later
                if (f_range == 0 || std::isnan(f_range))
                    f_range = 1.0;
                // Skip first and last interval
                for (ssize_t interval = 0; interval < n_intervals; interval++){
                    auto p = 2 * interval + 1; // NOTE: could cause problems, may have to subtract by 1
                    if (n_tot_refinements[interval] >= max_recursions){
                        // Skip intervals that have been refined too much
                        active[interval] = false;
                    } else if (std::isnan(fs[p-1]) || std::isnan(fs[p]) || std::isnan(fs[p+1])){
                        // If not all values fs[p-1], fs[p], fs[p+1] are finite, then set active interval to true
                        active[interval] = true;
                    } else {
                        double tot_w = 0.0;
                        // Do a small convolution
                        using ca_pair = std::pair<ssize_t, double>;
                        constexpr ca_pair _convarr[3] = {ca_pair(-1, 0.25), ca_pair(0, 0.5), ca_pair(1, 0.25)};
                        for (struct {std::size_t i; ssize_t q; double w;} c = {0, _convarr[0].first, _convarr[0].second}; c.i < 3; c.i++){ //auto [q, w] : _convarr
                            if (interval == 0 && c.q == -1)
                                continue;
                            if (interval == n_intervals-1 && c.q == 1)
                                continue;
                            tot_w += c.w;
                            ssize_t i = p + c.q;
                            // Estimate integral of second derivative over interval, use that as a refinement indicator
                            // https://mathformeremortals.wordpress.com/2013/01/12/a-numerical-second-derivative-from-three-points/
                            assert(i - 1 >= 0);
                            assert(i < fs.size());
                            assert(i + 1 < fs.size());
                            assert(i < xs.size());
                            assert(i + 1 < xs.size());

                            curvatures[interval] += std::abs(
                                2.0 *
                                (
                                    (fs[i + 1] - fs[i]) /
                                    ((xs[i + 1] - xs[i]) * (xs[i + 1] - xs[i - 1])) -
                                    (fs[i] - fs[i - 1]) /
                                    ((xs[i] - xs[i - 1]) * (xs[i + 1] - xs[i - 1]))
                                ) *
                                std::pow(xs[i + 1] - xs[i - 1], 2.0)
                            );
                        }
                        curvatures[interval] /= tot_w;
                        // Only consider intervals with a high enough curvature
                        active[interval] = curvatures[interval] > max_curvature;
                    }
                    
                }
                // Approximate end intervals as being the same curvature as those next to it.
                // This avoids computing the function in the end points
                curvatures[0] = curvatures[1];
                active[0] = active[1];
                curvatures[curvatures.size()-1] = curvatures[curvatures.size()-2];
                active[active.size()-1] = active[active.size()-2];

                bool will_break = true;
                decltype(n_tot_refinements) _n_tot_refinements_active;
                assert(active.size() == n_tot_refinements.size());
                for (std::size_t i = 0; i < n_tot_refinements.size(); i++){
                    if (active[i] && n_tot_refinements[i] < max_recursions){
                        will_break = false;
                        break;
                    }
                }
                if (will_break)
                    break;

                auto n_target_refinements = n_intervals / 2;
                std::vector<std::size_t> interval_candidates;
                assert(active.size() == n_intervals);
                for (std::size_t i = 0; i < n_intervals; i++){
                    if (active[i])
                        interval_candidates.push_back(i);
                }
                auto n_refinements = interval_candidates.size() < n_target_refinements ? 
                    interval_candidates.size() : n_target_refinements;
                decltype(curvatures) _curvatures_active;
                for (std::size_t i = 0; i < n_intervals; i++){
                    if (active[i])
                        _curvatures_active.push_back(curvatures[i]);
                }
                std::vector<std::size_t> perm = algext::sortperm(_curvatures_active);
                decltype(interval_candidates) intervals_to_refine;
                for (std::size_t i = perm.size() - n_refinements; i < perm.size(); i++){
                    intervals_to_refine.push_back(interval_candidates[perm[i]]);
                }
                std::sort(intervals_to_refine.begin(), intervals_to_refine.end());
                std::size_t n_intervals_to_refine = intervals_to_refine.size();
                std::size_t n_new_points = 2 * intervals_to_refine.size();

                // Do divison of the intervals
                decltype(xs) new_xs(n_points + n_new_points, 0.0);
                decltype(fs) new_fs(n_points + n_new_points, 0.0);
                decltype(n_tot_refinements) new_tot_refinements(n_intervals + n_intervals_to_refine, 0);
                std::size_t k = 0, kk = 0;
                for (std::size_t i = 0; i < n_points; i++){
                    if (i % 2 == 1){ // check if odd, because 0-indexed, not 1-indexed.
                        std::size_t interval = i / 2;
                        // if interval in intervals_to_refine
                        if (std::find(intervals_to_refine.begin(), intervals_to_refine.end(), interval)!=intervals_to_refine.end()){
                            kk += 1;
                            assert(interval < n_tot_refinements.size());
                            assert(interval - 1 + kk >= 0);
                            assert(interval - 1 + kk < new_tot_refinements.size());
                            assert(interval + kk < new_tot_refinements.size());
                            new_tot_refinements[interval - 1 + kk] = n_tot_refinements[interval] + 1;
                            new_tot_refinements[interval + kk] = n_tot_refinements[interval] + 1;

                            k += 1;
                            assert(i - 1 + k >= 0);
                            assert(i - 1 >= 0);
                            assert(i - 1 + k < new_xs.size());
                            assert(i + 1 + k < new_xs.size());
                            assert(i < xs.size());
                            
                            new_xs[i - 1 + k] = (xs[i] + xs[i - 1]) / 2.0;
                            new_fs[i - 1 + k] = giac_call(new_xs[i - 1 + k]);

                            new_xs[i + k] = xs[i];
                            new_fs[i + k] = fs[i];

                            new_xs[i + 1 + k] = (xs[i + 1] + xs[i]) / 2.0;
                            new_fs[i + 1 + k] = giac_call(new_xs[i + 1 + k]);
                            k += 1;
                        } else {
                            assert(interval + kk < new_tot_refinements.size());
                            assert(interval < n_tot_refinements.size());
                            assert(i + k < new_xs.size());
                            assert(i < xs.size());
                            new_tot_refinements[interval + kk] = n_tot_refinements[interval];
                            new_xs[i + k] = xs[i];
                            new_fs[i + k] = fs[i];
                        }
                    } else {
                        assert(i < xs.size());
                        assert(i + k < new_xs.size());
                        new_xs[i + k] = xs[i];
                        new_fs[i + k] = fs[i];
                    }
                }

                xs = std::move(new_xs);
                fs = std::move(new_fs);
                n_tot_refinements = std::move(new_tot_refinements);
                n_points += n_new_points;
                n_intervals = n_points / 2;
            }
            std::pair<decltype(xs), decltype(fs)> result(std::move(xs), std::move(fs));

            return std::move(result);
            #else
            return std::pair<std::vector<double>, std::vector<double>>(std::vector<double>(), std::vector<double>());
            #endif
    }

    Graph::Graph(lv_obj_t* parent){
        static lv_style_t zoom_style_bg;
        static lv_style_t zoom_style;
        static lv_style_t textarea_style;
        static lv_style_t function_button_style;

        offset = Point(VIEWPORT_WIDTH/2, -VIEWPORT_HEIGHT/2);
        scale = 1.0;
        VIEWPORT_HYP = calculate_hyp(VIEWPORT_WIDTH, VIEWPORT_HEIGHT);

        // canvas initialization
        canvas = lv_canvas_create(parent); 
        lv_canvas_set_buffer(canvas, buf, VIEWPORT_WIDTH, VIEWPORT_HEIGHT, LV_IMG_CF_TRUE_COLOR);
        lv_obj_align(canvas, LV_ALIGN_CENTER, 0, 0);
        lv_canvas_fill_bg(canvas, LV_COLOR_MAKE(255, 255, 255), LV_OPA_COVER);

        // zoom buttons backround initialization
        lv_style_init(&zoom_style_bg);
        lv_style_set_pad_all(&zoom_style_bg, 10);
        lv_style_set_border_width(&zoom_style_bg, 0);
        lv_style_set_bg_opa(&zoom_style_bg, LV_OPA_0);

        // zoom buttons foreground initialization
        lv_style_init(&zoom_style);
        lv_style_set_border_opa(&zoom_style, LV_OPA_50);
        lv_style_set_shadow_color(&zoom_style, lv_color_black());
        lv_style_set_shadow_spread(&zoom_style, 1);
        lv_style_set_shadow_width(&zoom_style, 20);
        lv_style_set_shadow_ofs_y(&zoom_style, 0);
        lv_style_set_shadow_opa(&zoom_style, LV_OPA_70);
        lv_style_set_text_font(&zoom_style, &lv_font_dejavu_16_persian_hebrew);

        // zoom button initialization
        zoom_buttons = lv_btnmatrix_create(canvas);
        lv_btnmatrix_set_map(zoom_buttons, function_button_map);
        lv_btnmatrix_set_btn_ctrl_all(zoom_buttons, LV_BTNMATRIX_CTRL_CLICK_TRIG);
        lv_obj_add_style(zoom_buttons, &zoom_style_bg, 0);
        lv_obj_add_style(zoom_buttons, &zoom_style, LV_PART_ITEMS);
        lv_obj_set_size(zoom_buttons, 80, 50);
        lv_obj_align(zoom_buttons, LV_ALIGN_TOP_RIGHT, 0, 0);
        lv_obj_add_event_cb(zoom_buttons, zoom_btnmatrix_cb, LV_EVENT_VALUE_CHANGED, this);

        // function_text_area style initialization
        lv_style_init(&textarea_style);
        lv_style_set_text_font(&textarea_style, &lv_font_montserrat_12_subpx);
        lv_style_set_border_width(&textarea_style, 1);
        lv_style_set_border_opa(&textarea_style, LV_OPA_50);
        lv_style_set_shadow_color(&textarea_style, lv_color_black());
        lv_style_set_shadow_spread(&textarea_style, 1);
        lv_style_set_shadow_width(&textarea_style, 20);
        lv_style_set_shadow_ofs_y(&textarea_style, 0);
        lv_style_set_shadow_opa(&textarea_style, LV_OPA_70);
        lv_style_set_pad_top(&textarea_style, 5);
        lv_style_set_pad_right(&textarea_style, 40);
        lv_style_set_text_align(&textarea_style, LV_ALIGN_LEFT_MID);


        // function_textarea initialization
        function_text_area = lv_textarea_create(canvas);
	Calculator::storeFunctionTA(function_text_area);
        lv_textarea_set_one_line(function_text_area, true);
        lv_obj_add_style(function_text_area, &textarea_style, 0);
        lv_obj_set_size(function_text_area, 180, 30);
        lv_obj_align(function_text_area, LV_ALIGN_TOP_MID, -20, 10);
        lv_obj_add_event_cb(function_text_area, textarea_cb, LV_EVENT_VALUE_CHANGED, this);

        // function_button style initialization
        lv_style_init(&function_button_style);
        lv_style_set_text_font(&function_button_style, &lv_font_montserrat_12_subpx);
        lv_style_set_text_align(&function_button_style, LV_ALIGN_CENTER);
        lv_style_set_pad_all(&function_button_style, 6);

        // function button initialization
        function_button = lv_dropdown_create(canvas);
        lv_dropdown_set_symbol(function_button , NULL);
        lv_obj_add_style(function_button, &function_button_style, 0);
        lv_obj_add_style(function_button, &function_button_style, LV_PART_ITEMS);
        lv_obj_set_size(function_button, 40, 30);
        lv_obj_align(function_button, LV_ALIGN_TOP_MID, 50, 10);
        lv_obj_add_event_cb(function_button, dropdown_button_cb, LV_EVENT_VALUE_CHANGED, this);

        #if 0
        // options button initialization
        options_button = lv_dropdown_create(canvas);
        lv_dropdown_set_options_static(options_button, options_button_text.c_str());
        lv_obj_add_style(options_button, &function_button_style, 0);
        lv_obj_add_style(options_button, &function_button_style, LV_PART_ITEMS);
        lv_obj_set_size(options_button, 40, 30);
        lv_obj_align(options_button, LV_ALIGN_TOP_LEFT, 5, 10);
        #endif

        // axes style initialization
        lv_draw_line_dsc_init(&axes_style);
        axes_style.color = LV_COLOR_MAKE(0, 0, 0);

        // canvas initialization
        lv_obj_add_flag(canvas, LV_OBJ_FLAG_CLICKABLE); // Allows for the canvas to be clickable and pressable
        lv_obj_add_event_cb(canvas, graph_event_cb, LV_EVENT_PRESSING, this);

        #if ENABLE_GIAC == 1
        giac::approx_mode(true, &ctx); // Change graphing to calculate approximate values.
        #endif
        add_function(""); // Add function f1 with no definition to the graph.
        draw_axes();
    }


    Point Graph::bottom_left_real() const {
        return Point{offset.x-real_width()/2, offset.y - real_height()/2};
    }

    Point Graph::bottom_right_real() const {
        return Point{offset.x+real_width()/2, offset.y - real_height()/2};
    }

    Point Graph::top_left_real() const {
        return Point{offset.x-real_width()/2, offset.y + real_height()/2};
    }

    Point Graph::top_right_real() const {
        return Point{offset.x+real_width()/2, offset.y + real_height()/2};
    }

    Point Graph::virtual_to_viewport(double x, double y) const {
            return Point{virtual_to_viewport_x(x), virtual_to_viewport_y(y)};
    }

    Point Graph::viewport_to_virtual(double x, double y) const {
            return Point{viewport_to_virtual_x(x), viewport_to_virtual_y(y)};//-(y+offset.y)*scale};
    }

    std::pair<double, double> Graph::viewport_virtual_domain() const {
            auto left = viewport_to_virtual(0, 0).x;
            auto right = viewport_to_virtual(VIEWPORT_WIDTH, 0).x;
            return std::pair<double, double>(left, right);
    }

    std::pair<double, double> Graph::viewport_virtual_range() const {
        auto top = viewport_to_virtual(0, 0).y;
        auto bottom = viewport_to_virtual(0, VIEWPORT_HEIGHT).y;
        return std::pair<double, double>(top, bottom);
    }

    #if ENABLE_GIAC
    template<typename T>
    T Graph::giac_call(std::string& func_name, T data) const {
        static char fcall_buf[128] = {0};
        constexpr int input_digits = 7;
        std::sprintf(fcall_buf, "%s(", func_name.c_str());
        if (std::is_same<T, double>::value){
            sprintf(fcall_buf+func_name.size()+1, "%.7lf)", data);
            giac::gen g(fcall_buf, &ctx);
            return (T)std::stod(giac::gen2string(giac::eval(g, &ctx)));
        } else {
            throw std::runtime_error("Unsupported type in giac_call.");
        }
    }
    #endif

    void Graph::draw_axes() {
            auto domain = viewport_virtual_domain();
            auto range = viewport_virtual_range();
            auto left_point = virtual_to_viewport(domain.first, 0.0);
            auto right_point = virtual_to_viewport(domain.second, 0.0);
            auto top_point = virtual_to_viewport(domain.first + offset.x, range.first);
            auto bottom_point = virtual_to_viewport(domain.first + offset.x, range.second);

            // points used to draw the horizontal and vertical axes.
            lv_point_t hpoints[] = {left_point.to_lv_point(), right_point.to_lv_point()};
            lv_point_t vpoints[] = {top_point.to_lv_point(), bottom_point.to_lv_point()};

            lv_canvas_draw_line(canvas, hpoints, 2, &axes_style);
            lv_canvas_draw_line(canvas, vpoints, 2, &axes_style);
        }

    void Graph::draw_ticks(){
        auto domain = viewport_virtual_domain();
        auto range = viewport_virtual_range();

        // constants used for defining tick text properties.
        const lv_coord_t tick_text_margin = 10;
        const lv_coord_t tick_text_pt = 10;

        // generate tick info based on the domain and range of the current viewport.
        Ticks::TickInfo hinfo = Ticks::optimize_ticks<2, 5, 3>(domain.first, domain.second);
        Ticks::TickInfo vinfo = Ticks::optimize_ticks<2, 5, 3>(range.second, range.first);

        // tick styling
        lv_draw_line_dsc_t ticks_style;
        lv_draw_line_dsc_init(&ticks_style);
        ticks_style.width = (lv_coord_t)1;
        ticks_style.opa = LV_OPA_30;
        lv_draw_label_dsc_t text_label;
        lv_draw_label_dsc_init(&text_label);
        text_label.font = &lv_font_montserrat_10; // make sure to change tick_text_pt is equal to the font's pt.

        // buffer for text.
        static char text_buf[64];

        static lv_point_t pts[2];
        for (auto& x : hinfo.ticks){
            auto vpx = (lv_coord_t)virtual_to_viewport_x(x);
            auto vpy1 = (lv_coord_t)virtual_to_viewport_y(range.first);
            auto vpy2 = (lv_coord_t)virtual_to_viewport_y(range.second);
            auto y0 = (lv_coord_t)virtual_to_viewport_y(0.0);

            if (y0 < (lv_coord_t)40 + tick_text_margin) { // upper bound for text
                y0 = (lv_coord_t)40 + tick_text_margin;
            } else if (y0 > (lv_coord_t)VIEWPORT_HEIGHT - tick_text_margin){ // lower bound for text
                y0 = (lv_coord_t)VIEWPORT_HEIGHT - tick_text_margin - tick_text_pt;
            } 
            
            pts[0] = lv_point_t{vpx, vpy1}; pts[1] = lv_point_t{vpx, vpy2};
            sprintf(text_buf, "%.2lf", x);
            lv_canvas_draw_line(canvas, pts, 2, &ticks_style); // draw vertical lines
            lv_canvas_draw_text(canvas, vpx, y0, 100, &text_label, text_buf); // draw text
        }

        for (auto& y : vinfo.ticks){
            if (y != 0.0){
                auto vpy = (lv_coord_t)virtual_to_viewport_y(y);
                auto vpx1 = (lv_coord_t)virtual_to_viewport_x(domain.first);
                auto vpx2 = (lv_coord_t)virtual_to_viewport_x(domain.second);
                auto x0 = (lv_coord_t)virtual_to_viewport_x(domain.first + offset.x);
                
                sprintf(text_buf, "%.2lf", y);
                int len = strlen(text_buf);
                lv_coord_t text_width = lv_txt_get_width(text_buf, len, text_label.font, text_label.letter_space, LV_TEXT_FLAG_NONE);
                
                if (x0 < tick_text_margin){ // left bound for text
                    x0 = tick_text_margin;
                } else if (x0 + len> (lv_coord_t)VIEWPORT_WIDTH - text_width - tick_text_margin){ // right bound for tex
                    x0 = (lv_coord_t)VIEWPORT_WIDTH - text_width - tick_text_margin;
                } 

                pts[0] = lv_point_t{vpx1, vpy}; pts[1] = lv_point_t{vpx2, vpy};
                
                lv_canvas_draw_line(canvas, pts, 2, &ticks_style); // draw horizontal lines
                lv_canvas_draw_text(canvas, x0, vpy, 100, &text_label, text_buf); // draw tex
            }
        }

    }

    /// If giac is enabled, the function is drawn onto the screen.
    /// If giac is disable, the function does nothing.
    void Graph::draw_function(int id, lv_color_t color){
        #if ENABLE_GIAC == 1
        if ((size_t)id >= plot_list.size())
            return;
        Plot& plot = plot_list[id];
        auto& func_name = plot.name;
        auto domain = viewport_virtual_domain();
        
        try {
            double center_x = (domain.first + domain.second)/2.0;
            plot.maybe_calculate(std::pair<double, double>(domain.first, domain.second), this->scale, center_x);
        } catch(...) {
            return;
        }

        auto segment_intersects_d = [](std::stack<Point>& stack, Point const& p0, Point const& p1, Point const& p2, Point const& p3){
            double s1_x, s1_y, s2_x, s2_y;
            // used because of float imprecision.
            constexpr double seg_eps = std::numeric_limits<double>::epsilon()*10.0;
            
            s1_x = p1.x - p0.x;   s1_y = p1.y - p0.y;
            s2_x = p3.x - p2.x;   s2_y = p3.y - p2.y;

            double det, s, t;
            det = (-s2_x * s1_y + s1_x * s2_y);
            if (det < seg_eps && det > -seg_eps) return false; // prevent division by zero.
            s = (-s1_y * (p0.x - p2.x) + s1_x * (p0.y - p2.y)) / det;
            t = ( s2_x * (p0.y - p2.y) - s2_y * (p0.x - p2.x)) / det;

            // intersection between segments.
            if (s >= 0 && s <= 1 && t >= 0 && t <= 1){
                stack.push(Point{p0.x + (t * s1_x), p0.y + (t * s1_y)});
                return true;
            }
            return false;
        };

        auto within_viewport_d = [=](Point point){
            return point.x >= 0.0 && point.x < (double)VIEWPORT_WIDTH && point.y >= 0.0 && point.y < (double)VIEWPORT_HEIGHT;
        };
        
        Option<lv_point_t> last_point = OptNone;
        Option<Point> last_point_f = OptNone;
        
        constexpr Point top_left_corner{0.0, 0.0};
        constexpr Point top_right_corner{(double)VIEWPORT_WIDTH, 0.0};
        constexpr Point bottom_right_corner{(double)VIEWPORT_WIDTH, (double)VIEWPORT_HEIGHT};
        constexpr Point bottom_left_corner{0.0, (double)VIEWPORT_HEIGHT};

        constexpr double coord_max = (double)(std::numeric_limits<lv_coord_t>::max()-1);
        const double mag_max = std::sqrt(coord_max*coord_max*2); // sqrt is not constexpr :(
        
        //lv_draw_rect_dsc_t rect_style;
        //lv_draw_rect_dsc_init(&rect_style);
        //rect_style.bg_color = LV_COLOR_MAKE(0,0,255); // debug point color.

        // Plotting algorithm:
        // 2 Steps:
        // -    Check to see if at least one point of the line segment is within the bounds of the viewport.
        //          If it is, then convert it to lv_point_t and plot it onto the screen.
        // -    If both points are outside of the viewport, check and see if it intersects the viewport.
        //          If it does, then plot the subsection of the segment that is within the bounds of the viewport.
        //          If it does not, then do not plot the line segment.
        
        std::stack<Point> ins_stack;
        #define POINT_SEG last_point_f.value(), curr_point_f
        #define SEG_INTERSECTS(corner1, corner2) segment_intersects_d(ins_stack, corner1, corner2, last_point_f.value(), curr_point_f)
        for (std::size_t i = 0; i < plot.cached_data.size(); i++) {
            auto& block = plot.cached_data[i];
            for(std::size_t j = 0; j < block.data.size(); j++) {
                auto& point = block.data[j];
                auto curr_point_f = virtual_to_viewport(point.x, point.y);
                if (!last_point_f.is_empty() && std::isfinite(curr_point_f.x) && std::isfinite(curr_point_f.y)){
                    bool wv_curr = within_viewport_d(curr_point_f);
                    bool wv_last = within_viewport_d(last_point_f.value());
                    // Fast check and plot if at least one point of the segment is within the viewport.
                    if (wv_last ^ wv_curr || (wv_last && wv_curr)){
                        lv_point_t line[2] = {last_point_f.value().to_lv_point(), curr_point_f.to_lv_point()};
                        lv_canvas_draw_line(canvas, line, 2, &plot.style);
                        //lv_canvas_draw_rect(canvas, line[0].x-2, line[0].y-2, 4, 4, &rect_style);
                        //lv_canvas_draw_rect(canvas, line[1].x-2, line[1].y-2, 4, 4, &rect_style);
                    } else {
                        bool intersects = false;
                        intersects |= SEG_INTERSECTS(top_left_corner, top_right_corner);
                        intersects |= SEG_INTERSECTS(top_right_corner, bottom_right_corner);
                        intersects |= SEG_INTERSECTS(bottom_right_corner, bottom_left_corner);
                        intersects |= SEG_INTERSECTS(bottom_left_corner, top_left_corner);
                        // if it is in this function, then the points of the segment are outside of the viewport,
                        if (ins_stack.size() == 2){ // unless in the corner, there should be two values.
                            lv_point_t ins_1 = ins_stack.top().to_lv_point(); ins_stack.pop();
                            lv_point_t ins_2 = ins_stack.top().to_lv_point(); ins_stack.pop();
                            lv_point_t line[2] = {ins_1, ins_2};
                            lv_canvas_draw_line(canvas, line, 2, &plot.style);
                            //lv_canvas_draw_rect(canvas, line[0].x-2, line[0].y-2, 4, 4, &rect_style);
                            //lv_canvas_draw_rect(canvas, line[1].x-2, line[1].y-2, 4, 4, &rect_style);
                        } else if (ins_stack.size() == 1){
                            ins_stack.pop();
                        }
                        
                    }
                }
                last_point_f = curr_point_f;
            }
        }

        #endif // giac end
    }

    
    void Graph::fill_background() {
        lv_canvas_fill_bg(canvas, LV_COLOR_MAKE(255, 255, 255), LV_OPA_COVER);
    }

    void Graph::update() {
        static char buf[20];
        lv_draw_label_dsc_t label;
        lv_draw_label_dsc_init(&label);
        label.font = &lv_font_montserrat_10;
        auto start = lv_tick_get();
        fill_background();
        draw_axes();
        for (size_t i = 0; i < plot_list.size(); i++){
            draw_function(i, lv_color_black());
        }
        draw_ticks();
        auto elapsed = lv_tick_elaps(start);
        #if 0
        // ms/f -> f/s
        std::sprintf(buf, "%.2lf", 1.0/(((double)elapsed)/1000.0));
        lv_canvas_draw_text(canvas, 10, 10, 100, &label, buf);
        #endif
    }

    std::string Graph::next_function_name(){
        return "f" + std::to_string(current_function_number++);
    }

    Plot* Graph::get_plot(std::string const& name) {
        for (size_t i = 0; i < plot_list.size(); i++){
            int comparison = plot_list[i].name.compare(name);
            if (comparison == 0){
                return &plot_list[i];  
            }
        }
        return nullptr;
    }

    Plot* Graph::get_plot(int id){
        if (id < 0 || (size_t)id >= plot_list.size())
            return nullptr;
        return &plot_list[id];
    }

    void Graph::add_function(std::string func){
        auto name = next_function_name();
        //auto id = get_newest_function_id_str();
        #if ENABLE_GIAC == 1
        giac::gen g(name + "(x):=" + std::move(func), &ctx);
        giac::eval(g, &ctx); 
        auto domain = viewport_virtual_domain();
        auto virtual_width = std::abs(domain.second - domain.first);
        Plot plot(std::move(name), std::move(func), get_next_color(), virtual_width, ctx);
        #else 
        Plot plot(std::move(name), std::move(func), lv_color_black());
        #endif
        plot_list.push_back(plot);
        switch_to_bold(plot_list.size()-1); // make this plot bold (as it is the one being added)
        if (function_button)
            lv_dropdown_set_options(function_button, get_plots_fmt_str().c_str());
    }

    void Graph::update_function(std::string func_def){
        std::cout << "UPDATE FUNCTION: " << func_def << "\n";
        #if ENABLE_GIAC == 1
        giac::gen g(func_def, &ctx);
        giac::eval(g,&ctx);
        #endif
    }
   

    std::string Graph::get_plots_fmt_str() const {
        std::string formatted = "";
        for (size_t i = 0; i < plot_list.size() - 1; i++){
            formatted += plot_list[i].name + "\n";
        }
        formatted += plot_list.back().name + "\n+";
        return formatted;
    }

    int Graph::get_newest_function_int() const{
        return current_function_number;
    }

    std::string Graph::get_newest_function_str() const{
        return "f" + std::to_string(get_newest_function_int()) + "(x)";
    }

    std::string Graph::get_newest_function_id_str() const{
        return "f" + std::to_string(get_newest_function_int());
    }

    void Graph::set_function_button(lv_obj_t* button){
        function_button = button;
    }

    void Graph::set_function_textarea(lv_obj_t* textarea){
        function_text_area = textarea;
    }

    void Graph::set_function_textarea_str(std::string const& text){
        if (!function_text_area)
            return;
        lv_textarea_set_text(function_text_area, text.c_str());
    }

    std::string Graph::get_function_button_selected_str() const{
        if (!function_button)
            return "";
        constexpr int buf_size = 10;
        char buf[buf_size];
        lv_dropdown_get_selected_str(function_button, buf, buf_size);
        return std::string(buf);
    }

    int Graph::get_function_button_selected_id() const{
        return (int)lv_dropdown_get_selected(function_button);
    }

    lv_color_t Graph::get_next_color() const{
        static size_t current = 0;
        if (current == default_colors.size())
            current = 0;
        return default_colors[current++];
    }

    void Graph::switch_to_bold(int id){
        static int current_bold = 0; // default to the first function being bold
        if ((size_t)id < plot_list.size()){
            plot_list[current_bold].style.width = DEFAULT_LINE_WIDTH;
            plot_list[id].style.width = BOLD_LINE_WIDTH;
            current_bold = id;
        }
    }
}