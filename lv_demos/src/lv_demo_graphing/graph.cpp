#include "graph.hxx"

namespace graphing {

    /// obj is an lv_canvas
    static void graph_event_cb(lv_event_t* event){
        static lv_point_t last_point{0,0};
        static auto last_time = lv_tick_get();
        if (lv_tick_elaps(last_time) > 100) {
            last_point = lv_point_t{0,0};
        }
        lv_hit_test_info_t* info = lv_event_get_hit_test_info(event);
        Graph* graph = static_cast<Graph*>(event->user_data);;
        lv_point_t point = *(info->point);
        //std::cout << "POINT: (" << point.x << " " << point.y << ")\n";
        if (last_point.x != 0 && last_point.y != 0){
            lv_point_t delta{point.x - last_point.x, point.y - last_point.y};
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
        lv_canvas_fill_bg(canvas, LV_COLOR_MAKE16(255, 255, 255), LV_OPA_COVER);

        lv_draw_line_dsc_init(&axes_style);
        axes_style.color = LV_COLOR_MAKE16(0, 0, 0);

        lv_obj_add_event_cb(canvas, graph_event_cb, LV_EVENT_HIT_TEST, this);

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

    Point Graph::real_to_viewport(mpf_class x, mpf_class y) const {
            return Point{(x+offset.x)/scale, (-y+offset.y)/scale + VIEWPORT_HEIGHT};
    }

    Point Graph::viewport_to_real(mpf_class x, mpf_class y) const {
            return Point{x*scale - offset.x, -(y-VIEWPORT_HEIGHT)*scale + offset.y};//-(y+offset.y)*scale};
    }

    std::pair<mpf_class, mpf_class> Graph::viewport_real_domain() const {
            auto left = viewport_to_real(0, 0).x;
            auto right = viewport_to_real(VIEWPORT_WIDTH, 0).x;
            return std::pair<mpf_class, mpf_class>(left, right);
    }

    std::pair<mpf_class, mpf_class> Graph::viewport_real_range() const {
        auto top = viewport_to_real(0, 0).y;
        auto bottom = viewport_to_real(0, VIEWPORT_HEIGHT).y;
        return std::pair<mpf_class, mpf_class>(top, bottom);
    }

    void Graph::draw_axes() {
            auto domain = viewport_real_domain();
            auto range = viewport_real_range();
            auto left_point = real_to_viewport(domain.first, CREATE_MPF("0"));
            auto right_point = real_to_viewport(domain.second, CREATE_MPF("0"));
            auto top_point = real_to_viewport(domain.first + offset.x, range.first);
            auto bottom_point = real_to_viewport(domain.first + offset.x, range.second);

            //std::cout << "DOMAIN: ("<<domain.first.get_d()<<" "<<domain.second.get_d()<<")\n";
            //std::cout << "RANGE: ("<<range.second.get_d()<<" "<<range.first.get_d()<<")\n";

            lv_point_t hpoints[] = {left_point.to_lv_point(), right_point.to_lv_point()};
            lv_point_t vpoints[] = {top_point.to_lv_point(), bottom_point.to_lv_point()};
            

            mpf_class hmarker_width = real_width()/8, vmarker_width = real_height()/8;
            
            for (mpf_class hx = domain.first; hx <= domain.second; hx += hmarker_width){
                auto rp1 = real_to_viewport(hx, CREATE_MPF("2")).to_lv_point();
                auto rp2 = real_to_viewport(hx, CREATE_MPF("-2")).to_lv_point();
                //std::cout << hx.get_d() << " => " << rp1.x << "\n"; 
                lv_point_t pts[] = {rp1, rp2};
                lv_canvas_draw_line(canvas, pts, 2, &axes_style);
            }
            //std::cout << "----\n";
            
            for (mpf_class vy = range.second; vy <= range.first; vy += vmarker_width){
                auto rp1 = real_to_viewport(domain.first + offset.x + CREATE_MPF("-2"), vy).to_lv_point();
                auto rp2 = real_to_viewport(domain.first + offset.x + CREATE_MPF("2"), vy).to_lv_point();
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
        if (id >= plot_list.size())
            return;
        Plot& plot = plot_list[id];
        auto& func_name = plot.name;
        const int number_of_points = 1000;

        auto giac_call = [&](auto a){
            std::stringstream ss, ss2;
            ss << func_name << "(" << a << ")";
            giac::gen g(ss.str(), &ctx);
            //std::cout << "GIAC_CALL: " << ss.str() << "\n";
            ss2 << giac::eval(g, &ctx);
            //std::cout << "RESULT:" << ss2.str() << "\n";
            return mpf_class(ss2.str());
        };

        auto domain = viewport_real_domain();
        auto range = viewport_real_range();
        mpf_class step = (domain.second - domain.first)/number_of_points;
        static lv_point_t vals[number_of_points];
        mpf_class acc = domain.first;    
        try {
            for (int i = 0; i < number_of_points; i++){
            mpf_class val = giac_call(acc);
            Point vpoint = real_to_viewport(acc, val);
            lv_point_t point = vpoint.to_lv_point();
            vals[i] = point;
            acc += step;
            }
        } catch (...){
            return;
        }
        
        lv_canvas_draw_line(canvas, vals, number_of_points, &plot.style);
        #endif
    }

    
    void Graph::fill_background() {
        lv_canvas_fill_bg(canvas, LV_COLOR_MAKE16(255, 255, 255), LV_OPA_COVER);
    }

    void Graph::update() {
        fill_background();
        draw_axes();
        for (int i = 0; i < plot_list.size(); i++){
            draw_function(i, lv_color_black());
        }
    }

    std::string Graph::next_function_name(){
        return "f" + std::to_string(current_function_number++);
    }

    Plot* Graph::get_plot(std::string const& name) {
        for (auto i = 0; i < plot_list.size(); i++){
            int comparison = plot_list[i].name.compare(name);
            if (comparison == 0){
                return &plot_list[i];  
            }
        }
        return nullptr;
    }

    Plot* Graph::get_plot(int id){
        if (id < 0 || id >= plot_list.size())
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
        for (int i = 0; i < plot_list.size() - 1; i++){
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
        static int current = 0;
        if (current == default_colors.size())
            current = 0;
        return default_colors[current++];
    }

    void Graph::switch_to_bold(int id){
        static int current_bold = 0; // default to the first function being bold
        if (id < plot_list.size()){
            plot_list[current_bold].style.width = DEFAULT_LINE_WIDTH;
            plot_list[id].style.width = BOLD_LINE_WIDTH;
            current_bold = id;
        }
    }
}