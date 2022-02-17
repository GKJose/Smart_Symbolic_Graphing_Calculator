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
            //offset = Point("0", "0");
            scale = CREATE_MPF("1");
            VIEWPORT_HYP = calculate_hyp(VIEWPORT_WIDTH, VIEWPORT_HEIGHT);

            canvas = lv_canvas_create(parent);
            lv_canvas_set_buffer(canvas, buf, VIEWPORT_WIDTH, VIEWPORT_HEIGHT, LV_IMG_CF_TRUE_COLOR);
            lv_obj_align(canvas, LV_ALIGN_CENTER, 0, 0);
            lv_canvas_fill_bg(canvas, LV_COLOR_MAKE16(255, 255, 255), LV_OPA_COVER);

            lv_draw_line_dsc_init(&axes_style);
            axes_style.color = LV_COLOR_MAKE16(0, 0, 0);

            lv_obj_add_event_cb(canvas, graph_event_cb, LV_EVENT_HIT_TEST, this);

            //lv_draw_line_dsc_t middlerino;
            //lv_draw_line_dsc_init(&middlerino);
            //middlerino.color = LV_COLOR_MAKE16(255, 0, 0);

            draw_axes();
            
            //lv_canvas_draw_line(canvas, middle, 2, &middlerino);
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
            //auto blr = bottom_left_real();
            //auto tlr = top_left_real();
            //return Point{(x - blr.x)*scale, VIEWPORT_HEIGHT - (blr.y - y)*scale};
            return Point{(x+offset.x), -(y-offset.y) + VIEWPORT_HEIGHT};
    }

    Point Graph::viewport_to_real(mpf_class x, mpf_class y) const {
            //auto blr = bottom_left_real();
            //return Point{blr.x + x/scale, blr.y + (VIEWPORT_HEIGHT - y)/scale};
            return Point{-(x+offset.x), -(y+offset.y)};
    }

    std::pair<mpf_class, mpf_class> Graph::viewport_real_domain() const {
            // mpf_class left_x = center.x - VIEWPORT_WIDTH/2;
            // mpf_class right_x = center.x + VIEWPORT_WIDTH/2;
            auto left = viewport_to_real(0, 0).x;
            auto right = viewport_to_real(VIEWPORT_WIDTH, 0).x;
            return std::pair<mpf_class, mpf_class>(left, -right - 2*offset.x);
            //return std::pair<mpf_class, mpf_class>(-offset.x, 2*offset.x);
    }

    std::pair<mpf_class, mpf_class> Graph::viewport_real_range() const {
        // mpf_class top_y = center.y - VIEWPORT_HEIGHT/2;
        // mpf_class bottom_y = center.y + VIEWPORT_HEIGHT/2;
        return std::pair<mpf_class, mpf_class>(offset.y + VIEWPORT_HEIGHT, offset.y);
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

    void Graph::draw_function(graph_function func){
        const int number_of_points = 1000;
        auto domain = viewport_real_domain();
        auto range = viewport_real_range();
        mpf_class step = (domain.second - domain.first)/number_of_points;
        static lv_point_t vals[number_of_points];
        mpf_class acc = domain.first;
        //std::cout << "domain: <" << domain.first.get_d() << " " << domain.second.get_d() << ">\n";
        for (int i = 0; i < number_of_points; i++){
            mpf_class val = func(acc);
            //std::cout << "VAL: " << val.get_d() << "\n";
            Point vpoint = real_to_viewport(acc, val);
            //std::cout << vpoint.to_string() << "\n";
            //std::cout << "VPOINT: (" << vpoint.x.get_d() << " " << vpoint.y.get_d() << ")\n";
            lv_point_t point = vpoint.to_lv_point();
            //std::cout << "(" << point.x << " " << point.y << ")\n";
            vals[i] = point;
            acc += step;
        }
        lv_canvas_draw_line(canvas, vals, number_of_points, &axes_style);
    }

    void Graph::draw_function(Plot const& plot){
        const int number_of_points = 1000;
        auto domain = viewport_real_domain();
        auto range = viewport_real_range();
        mpf_class step = (domain.second - domain.first)/number_of_points;
        static lv_point_t vals[number_of_points];
        mpf_class acc = domain.first;
        //std::cout << "domain: <" << domain.first.get_d() << " " << domain.second.get_d() << ">\n";
        for (int i = 0; i < number_of_points; i++){
            mpf_class val = plot.func(acc);
            //std::cout << "VAL: " << val.get_d() << "\n";
            Point vpoint = real_to_viewport(acc, val);
            //std::cout << vpoint.to_string() << "\n";
            //std::cout << "VPOINT: (" << vpoint.x.get_d() << " " << vpoint.y.get_d() << ")\n";
            lv_point_t point = vpoint.to_lv_point();
            //std::cout << "(" << point.x << " " << point.y << ")\n";
            vals[i] = point;
            acc += step;
        }
        lv_canvas_draw_line(canvas, vals, number_of_points, &plot.style);
    }

    void Graph::draw_function(graph_function func, lv_color_t color){
        draw_function(Plot(func, color));
    }

    
    void Graph::fill_background() {
        lv_canvas_fill_bg(canvas, LV_COLOR_MAKE16(255, 255, 255), LV_OPA_COVER);
    }

    void Graph::update() {
        fill_background();
        draw_axes();
        for (int i = 0; i < plot_list.size(); i++){
            draw_function(plot_list[i]);
        }
    }

    void Graph::add_function(graph_function func) {
        plot_list.push_back(Plot(func, axes_style.color));
    }

    void Graph::add_function(Plot const& plot) {
        plot_list.push_back(plot);
    }

    void Graph::add_function(graph_function func, lv_color_t color){
        add_function(Plot(func, color));
    }
}