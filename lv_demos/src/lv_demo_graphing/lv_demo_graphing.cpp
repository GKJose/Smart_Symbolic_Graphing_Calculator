#include "lv_demo_graphing.hxx"
#include "graph.hxx"
#include <math.h>

static mpf_class plot_sin(mpf_class x) {
    return mpf_class(sin(x.get_d()*0.1) * 50.0);
}

static void zoom_btnmatrix_cb(lv_event_t* event){
    uint32_t id = lv_btnmatrix_get_selected_btn(event->target);
    if (id == 0) {
        static_cast<graphing::Graph*>(event->user_data)->scale_delta(mpf_class(-0.1));
    } else {
        static_cast<graphing::Graph*>(event->user_data)->scale_delta(mpf_class(+0.1));
    }
}

void create_graph(void){
    static graphing::Graph graph(lv_scr_act());
    static lv_style_t zoom_style_bg;
    static lv_style_t zoom_style;
    static const char* map[] = {"+", "-"};
    static lv_obj_t* zoom_buttons = lv_btnmatrix_create(graph.get_canvas());

    lv_style_init(&zoom_style_bg);
    lv_style_set_pad_all(&zoom_style_bg, 10);
    //lv_style_set_pad_gap(&zoom_style_bg, 0);
    //lv_style_set_clip_corner(&zoom_style_bg, true);
    //lv_style_set_radius(&zoom_style_bg, LV_RADIUS_CIRCLE);
    lv_style_set_border_width(&zoom_style_bg, 0);
    lv_style_set_bg_opa(&zoom_style_bg, LV_OPA_0);
    //lv_style_set_size(&zoom_style_bg, 50);

    lv_style_init(&zoom_style);
    //lv_style_set_radius(&zoom_style, 0);
    //lv_style_set_border_width(&zoom_style, 1);
    lv_style_set_border_opa(&zoom_style, LV_OPA_50);
    lv_style_set_shadow_color(&zoom_style, lv_color_black());
    lv_style_set_shadow_spread(&zoom_style, 1);
    lv_style_set_shadow_width(&zoom_style, 20);
    lv_style_set_shadow_ofs_y(&zoom_style, 0);
    lv_style_set_shadow_opa(&zoom_style, LV_OPA_70);
    //lv_style_set_border_color(&zoom_style, lv_palette_main(LV_PALETTE_GREY));
    //lv_style_set_border_side(&zoom_style, LV_BORDER_SIDE_INTERNAL);
    //lv_style_set_radius(&zoom_style, 0);

    lv_btnmatrix_set_map(zoom_buttons, map);
    lv_btnmatrix_set_btn_ctrl_all(zoom_buttons, LV_BTNMATRIX_CTRL_CLICK_TRIG);
    lv_obj_add_style(zoom_buttons, &zoom_style_bg, 0);
    lv_obj_add_style(zoom_buttons, &zoom_style, LV_PART_ITEMS);
    lv_obj_set_size(zoom_buttons, 80, 40);
    lv_obj_align(zoom_buttons, LV_ALIGN_TOP_RIGHT, 0, 0);
    lv_obj_add_event_cb(zoom_buttons, zoom_btnmatrix_cb, LV_EVENT_VALUE_CHANGED, &graph);
   

    graph.set_scale(mpf_class("0.5"));
    graph.add_function(plot_sin, LV_COLOR_MAKE16(255, 0, 0));
    graph.add_function([](mpf_class x){return x;}, LV_COLOR_MAKE16(0, 255, 0));
    graph.update(); 
}


