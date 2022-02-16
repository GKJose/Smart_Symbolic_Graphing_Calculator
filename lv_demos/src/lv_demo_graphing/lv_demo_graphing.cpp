#include "lv_demo_graphing.hxx"
#include "graph.hxx"
#include <math.h>

static mpf_class plot_sin(mpf_class x) {
    return mpf_class(sin(x.get_d()*0.1) * 50.0);
}

void create_graph(void){

    graphing::Graph graph(lv_scr_act());
    //graph.set_scale(mpf_class(1.5));
    graph.translate_center(0,0);

    //mpf_class m("0.5");
    //mp_exp_t e(1);
    //std::cout << plot_sin(m).get_d() << "\n";
    graph.draw_function(plot_sin);
    //graph.draw_function([](mpf_class x){return x;});
    //graph.draw_function([](mpf_class x){ return mpf_class(sin(x.get_d()) * 10.0);});
    //graph.translate_center(25, 25);

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


