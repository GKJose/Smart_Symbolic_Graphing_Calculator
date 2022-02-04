#include "lv_demo_graphing.hxx"
#include <string>

#define GRAPH_BUF_DIMS LV_CANVAS_BUF_SIZE_TRUE_COLOR(MONITOR_HOR_RES, MONITOR_VER_RES)

static lv_color_t graph_buf1[GRAPH_BUF_DIMS];
static lv_color_t graph_buf2[GRAPH_BUF_DIMS];


namespace lv_demo_graphing {

    Graph::Graph(lv_obj_t* parent){
            center = Point("0", "0");
            scale = CREATE_MPF("1");

            canvas = lv_canvas_create(parent);
            lv_canvas_set_buffer(canvas, buf, VIEWPORT_WIDTH, VIEWPORT_HEIGHT, LV_IMG_CF_TRUE_COLOR);
            lv_obj_align(canvas, LV_ALIGN_CENTER, 0, 0);
            lv_canvas_fill_bg(canvas, LV_COLOR_MAKE16(255, 255, 255), LV_OPA_COVER);

            lv_draw_line_dsc_init(&axes_style);
            axes_style.color = LV_COLOR_MAKE16(0, 0, 0);

            draw_axes();
    }

    Point Graph::bottom_left_real() const {
        return Point{center.x-real_width()/2, center.y - real_height()/2};
    }

    Point Graph::bottom_right_real() const {
        return Point{center.x+real_width()/2, center.y - real_height()/2};
    }

    Point Graph::top_left_real() const {
        return Point{center.x-real_width()/2, center.y + real_height()/2};
    }

    Point Graph::top_right_real() const {
        return Point{center.x+real_width()/2, center.y + real_height()/2};
    }

    Point Graph::real_to_viewport(mpf_class x, mpf_class y) const {
            //auto blr = bottom_left_real();
            auto tlr = top_left_real();
            //return Point{(x - blr.x)*scale, VIEWPORT_HEIGHT - (blr.y - y)*scale};
            return Point{(x-tlr.x)*scale, (tlr.y-y)*scale};
    }

    Point Graph::viewport_to_real(mpf_class x, mpf_class y) const {
            auto blr = bottom_left_real();
            return Point{blr.x + x/scale, blr.y + (VIEWPORT_HEIGHT - y)/scale};
    }

    std::pair<mpf_class, mpf_class> Graph::viewport_real_domain() const {
            auto left_x = viewport_to_real(CREATE_MPF("0"), VIEWPORT_HEIGHT).x;
            auto right_x = viewport_to_real(VIEWPORT_WIDTH, VIEWPORT_HEIGHT).x;
            return std::pair<mpf_class, mpf_class>(left_x, right_x);
    }

    std::pair<mpf_class, mpf_class> Graph::viewport_real_range() const {
        auto top_y = top_left_real().y;
        auto bottom_y = bottom_left_real().y;
        return std::pair<mpf_class, mpf_class>(top_y, bottom_y);
    }

    void Graph::draw_axes() {
            auto domain = viewport_real_domain();
            auto range = viewport_real_range();
            auto left_point = real_to_viewport(domain.first, CREATE_MPF("0"));
            auto right_point = real_to_viewport(domain.second, CREATE_MPF("0"));
            auto top_point = real_to_viewport(CREATE_MPF("0"), range.first);
            auto bottom_point = real_to_viewport(CREATE_MPF("0"), range.second);
            
            lv_point_t hpoint1{left_point.x.get_si(), left_point.y.get_si()};
            lv_point_t hpoint2{right_point.x.get_si(), right_point.y.get_si()};
            lv_point_t hpoints[] = {hpoint1, hpoint2};

            lv_point_t vpoint1{top_point.x.get_si(), top_point.y.get_si()};
            lv_point_t vpoint2{bottom_point.x.get_si(), bottom_point.y.get_si()};
            lv_point_t vpoints[] = {vpoint1, vpoint2};

            mpf_class hmarker_width = real_width()/8, vmarker_width = real_height()/8;
            
            for (mpf_class hx = domain.first; hx <= domain.second; hx += hmarker_width){
                auto rp1 = real_to_viewport(hx, CREATE_MPF("0"));
                auto rp2 = real_to_viewport(hx, CREATE_MPF("0"));
                lv_point_t p1{rp1.x.get_si(), rp1.y.get_si() + 2 };
                lv_point_t p2{rp2.x.get_si(), rp2.y.get_si() - 2 };
                lv_point_t pts[] = {p1, p2};
                lv_canvas_draw_line(canvas, pts, 2, &axes_style);
            }

            
            for (mpf_class vy = range.second; vy <= range.first; vy += vmarker_width){
                auto rp = real_to_viewport(CREATE_MPF("0"), vy);
                lv_point_t p1{rp.x.get_si() - 2, rp.y.get_si()};
                lv_point_t p2{rp.x.get_si() + 2, rp.y.get_si()};
                lv_point_t pts[] = {p1, p2};
                lv_canvas_draw_line(canvas, pts, 2, &axes_style);
            }


            lv_canvas_draw_line(canvas, hpoints, 2, &axes_style);
            lv_canvas_draw_line(canvas, vpoints, 2, &axes_style);
        }
    
}


void lv_demo_graphing::create_graph(void){

    Graph graph(lv_scr_act());
    

    // lv_obj_t* canvas = lv_canvas_create(lv_scr_act());
    


    // lv_draw_label_dsc_t label;
    // lv_draw_label_dsc_init(&label);
    // label.color = LV_COLOR_MAKE16(255, 255, 255);
    // label.font = &lv_font_montserrat_12_subpx;
    

    // lv_canvas_set_buffer(canvas, graph_buf1, MONITOR_HOR_RES, MONITOR_VER_RES, LV_IMG_CF_TRUE_COLOR);
    // lv_obj_align(canvas, LV_ALIGN_CENTER, 0, 0);
    // lv_canvas_fill_bg(canvas, LV_COLOR_MAKE16(128, 128, 128), LV_OPA_COVER);
    // lv_canvas_draw_text(canvas, 30, 30, 100, &label, "Greetings fellow travelers!");

    // lv_point_t point1{3,3};
    // lv_point_t point2{18,18};
    // lv_point_t points[] = {point1, point2};
    // lv_draw_line_dsc_t stylerino;
    // lv_draw_line_dsc_init(&stylerino);
    // stylerino.color = LV_COLOR_MAKE16(255, 255, 255);

    // lv_canvas_draw_line(canvas, points, 2, &stylerino);
    
}


