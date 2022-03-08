#include "graph.hxx"
#include "algorithm_extensions.hxx"
#include <random>
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
        }
        last_point = point;
        last_time = lv_tick_get();
    }

    Graph::Graph(lv_obj_t* parent){
        offset = Point(VIEWPORT_WIDTH/2, -VIEWPORT_HEIGHT/2);
        scale = CREATE_MPF("1");
        VIEWPORT_HYP = calculate_hyp(VIEWPORT_WIDTH, VIEWPORT_HEIGHT);

        canvas = lv_canvas_create(parent); 
        lv_canvas_set_buffer(canvas, buf, VIEWPORT_WIDTH, VIEWPORT_HEIGHT, LV_IMG_CF_TRUE_COLOR);
        lv_obj_align(canvas, LV_ALIGN_CENTER, 0, 0);
        lv_canvas_fill_bg(canvas, LV_COLOR_MAKE(255, 255, 255), LV_OPA_COVER);

        lv_draw_line_dsc_init(&axes_style);
        axes_style.color = LV_COLOR_MAKE(0, 0, 0);

        lv_obj_add_flag(canvas, LV_OBJ_FLAG_CLICKABLE); // Allows for the canvas to be clickable and pressable
        lv_obj_add_event_cb(canvas, graph_event_cb, LV_EVENT_PRESSING, this);
        #if ENABLE_GIAC == 1
        giac::approx_mode(true, &ctx); // Change graphing to calculate approximate values.
        #endif

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

    Point Graph::virtual_to_viewport(mpf_class x, mpf_class y) const {
            return Point{virtual_to_viewport_x(x), virtual_to_viewport_y(y)};
    }

    Point Graph::viewport_to_virtual(mpf_class x, mpf_class y) const {
            return Point{viewport_to_virtual_x(x), viewport_to_virtual_y(y)};//-(y+offset.y)*scale};
    }

    std::pair<mpf_class, mpf_class> Graph::viewport_virtual_domain() const {
            auto left = viewport_to_virtual(0, 0).x;
            auto right = viewport_to_virtual(VIEWPORT_WIDTH, 0).x;
            return std::pair<mpf_class, mpf_class>(left, right);
    }

    std::pair<mpf_class, mpf_class> Graph::viewport_virtual_range() const {
        auto top = viewport_to_virtual(0, 0).y;
        auto bottom = viewport_to_virtual(0, VIEWPORT_HEIGHT).y;
        return std::pair<mpf_class, mpf_class>(top, bottom);
    }

    void Graph::draw_axes() {
            auto domain = viewport_virtual_domain();
            auto range = viewport_virtual_range();
            auto left_point = virtual_to_viewport(domain.first, CREATE_MPF("0"));
            auto right_point = virtual_to_viewport(domain.second, CREATE_MPF("0"));
            auto top_point = virtual_to_viewport(domain.first + offset.x, range.first);
            auto bottom_point = virtual_to_viewport(domain.first + offset.x, range.second);

            //std::cout << "DOMAIN: ("<<domain.first.get_d()<<" "<<domain.second.get_d()<<")\n";
            //std::cout << "RANGE: ("<<range.second.get_d()<<" "<<range.first.get_d()<<")\n";

            lv_point_t hpoints[] = {left_point.to_lv_point(), right_point.to_lv_point()};
            lv_point_t vpoints[] = {top_point.to_lv_point(), bottom_point.to_lv_point()};

            const auto marker_length_px = 10;
            const mpf_class marker_length_virtual = viewport_to_virtual_x(marker_length_px) - viewport_to_virtual_x(0);

            mpf_class hmarker_width = real_width()/8, vmarker_width = real_height()/8;
            
            for (mpf_class hx = domain.first; hx <= domain.second; hx += hmarker_width){
                auto rp1 = virtual_to_viewport(hx, marker_length_virtual/2).to_lv_point();
                auto rp2 = virtual_to_viewport(hx, -marker_length_virtual/2).to_lv_point();
                //std::cout << hx.get_d() << " => " << rp1.x << "\n"; 
                lv_point_t pts[] = {rp1, rp2};
                lv_canvas_draw_line(canvas, pts, 2, &axes_style);
            }
            //std::cout << "----\n";
            
            for (mpf_class vy = range.second; vy <= range.first; vy += vmarker_width){
                auto rp1 = virtual_to_viewport(domain.first + offset.x + -marker_length_virtual/2, vy).to_lv_point();
                auto rp2 = virtual_to_viewport(domain.first + offset.x + marker_length_virtual/2, vy).to_lv_point();
                //std::cout << vy.get_d() << " => " << rp1.y << "\n"; 
                lv_point_t pts[] = {rp1, rp2};
                lv_canvas_draw_line(canvas, pts, 2, &axes_style);
            }

            lv_canvas_draw_line(canvas, hpoints, 2, &axes_style);
            lv_canvas_draw_line(canvas, vpoints, 2, &axes_style);
        }

    /// If giac is enabled, the function is drawn onto the screen.
    /// If giac is disable, the function does nothing.
    void Graph::draw_function(int id, lv_color_t color){
        #if ENABLE_GIAC == 1
        if ((size_t)id >= plot_list.size())
            return;
        Plot& plot = plot_list[id];
        auto& func_name = plot.name;
        const int number_of_points = 1000;
        const int input_digits = 7;
        static char fcall_buf[128] = {0};
        std::sprintf(fcall_buf, "%s(", func_name.c_str());
        auto giac_call = [&](auto a) -> mpf_class{
            gmp_sprintf(fcall_buf+func_name.size()+1, "%.*Ff)", input_digits, a);
            giac::gen g(fcall_buf, &ctx);
            return mpf_class(giac::gen2string(giac::eval(g, &ctx)));
        };
        auto domain = viewport_virtual_domain();
        auto range = viewport_virtual_range();

        #if ENABLE_EXPERIMENTAL_PLOTTING
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
        const double max_curvature = 2;

        std::size_t n_points = 21;
        std::size_t n_intervals = n_points / 2;
        assert(n_points % 2 == 1); // assert that n_points is odd

        std::vector<double> xs = algext::linspace(domain.first.get_d(), domain.second.get_d(), n_points);
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
            try {
              fs[i] = giac_call(mpf_class(xs[i])).get_d();
            } catch(...){
                return;
            }
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
                    for (auto [q, w] : _convarr){
                        if (interval == 0 && q == -1)
                            continue;
                        if (interval == n_intervals-1 && q == 1)
                            continue;
                        tot_w += w;
                        ssize_t i = p + q;
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
                        new_fs[i - 1 + k] = giac_call(mpf_class(new_xs[i - 1 + k])).get_d();

                        new_xs[i + k] = xs[i];
                        new_fs[i + k] = fs[i];

                        new_xs[i + 1 + k] = (xs[i + 1] + xs[i]) / 2.0;
                        new_fs[i + 1 + k] = giac_call(mpf_class(new_xs[i + 1 + k])).get_d();
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
        lv_draw_rect_dsc_t rect_style;
        lv_draw_rect_dsc_init(&rect_style);
        rect_style.bg_color = plot.style.color;
        std::vector<lv_point_t> vals;
        vals.reserve(xs.size());
        vals.clear();
        for (std::size_t i = 0; i < xs.size(); i++){
            Point temp = virtual_to_viewport(mpf_class(xs[i]), mpf_class(fs[i]));
            if (temp.x < 0 || temp.x > VIEWPORT_WIDTH || temp.y < 0 || temp.y > VIEWPORT_HEIGHT){
                if (vals.size() >= 2){
                    #if ENABLE_DBGPLOT
                    for (auto& v : vals){
                        lv_canvas_draw_rect(canvas, v.x-2, v.y-2, 2, 2, &rect_style);
                    }
                    #else
                    lv_canvas_draw_line(canvas, vals.data(), vals.size(), &plot.style);
                    #endif
                }
                vals.clear();
            }
            vals.push_back(temp.to_lv_point());
            //vals[i] = virtual_to_viewport(mpf_class(xs[i]), mpf_class(fs[i])).to_lv_point();
        }
        if (vals.size() > 0){
            #if ENABLE_DBGPLOT
            for (auto& v : vals){
                lv_canvas_draw_rect(canvas, v.x-2, v.y-2, 2, 2, &rect_style);
            }
            #else
            lv_canvas_draw_line(canvas, vals.data(), vals.size(), &plot.style);
            #endif
        }
        #else // Brute Force Plotting (always plotting 2000 points).
        
        mpf_class step = (domain.second - domain.first)/number_of_points;
        static lv_point_t vals[number_of_points];
        mpf_class acc = domain.first;    
        try {
            for (int i = 0; i < number_of_points; i++){
            mpf_class val = giac_call(acc);
            Point vpoint = virtual_to_viewport(acc, val);
            lv_point_t point = vpoint.to_lv_point();
            vals[i] = point;
            acc += step;
            }
        } catch (...){
            return;
        }
        lv_canvas_draw_line(canvas, vals, number_of_points, &plot.style);
        #endif // experimental plotting end
        #endif // giac end
    }

    
    void Graph::fill_background() {
        lv_canvas_fill_bg(canvas, LV_COLOR_MAKE(255, 255, 255), LV_OPA_COVER);
    }

    void Graph::update() {
        fill_background();
        draw_axes();
        for (size_t i = 0; i < plot_list.size(); i++){
            draw_function(i, lv_color_black());
        }
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
        #endif
        Plot plot(std::move(name), std::move(func), lv_color_black());
        plot.style.color = get_next_color();
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