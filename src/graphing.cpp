#include <graphing.hxx>
#include <graph.hxx>
#include <math.h>

static lv_obj_t* function_text_area;

static mpf_class plot_sin(mpf_class x) {
    return mpf_class(sin(x.get_d()*0.1) * 50.0);
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

static void dropdown_button_cb(lv_event_t* event){
    char buf[10];
    std::cout << "DROPDOWN_BUTTON_CB\n";
    //std::stringstream ss;
    int id = lv_dropdown_get_selected(event->target);
    lv_dropdown_get_selected_str(event->target, buf, sizeof(buf)); // C function, so no std::string
    //ss << buf;
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
        // std::cout << "ID: " << id << "\n";
        // std::cout << "EXPR: " << plot->function_expression << "\n";
        graph->set_function_textarea_str(plot->function_expression);
    }
        
    graph->update();
    //std::cout << buf << "\n";
}

static void textarea_cb(lv_event_t* event){
    auto graph = static_cast<graphing::Graph*>(event->user_data);
    auto func_str = graph->get_function_button_selected_str();
    auto func_id = graph->get_function_button_selected_id();
    auto func_expression = std::string(lv_textarea_get_text(event->target));
    auto plot = graph->get_plot(func_id);
    std::cout << "TEXTAREA_CB: " << func_str << " " << func_expression << " " << func_id << "\n";
    graph->update_function(func_str + "(x):=" + func_expression);
    plot->name = std::move(func_str);
    plot->change_expression(std::move(func_expression));
    graph->update();  
}

void create_graph(lv_obj_t *parent){
    static graphing::Graph graph(parent);
    static lv_style_t zoom_style_bg;
    static lv_style_t zoom_style;
    static lv_style_t textarea_style;
    static lv_style_t function_button_style;
    static const char* map[] = {"+", "-", ""};
    static lv_obj_t* zoom_buttons = lv_btnmatrix_create(graph.get_canvas());
    function_text_area = lv_textarea_create(graph.get_canvas());
    static lv_obj_t* function_button = lv_dropdown_create(graph.get_canvas());
    // zoom button setup

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
    lv_style_set_text_font(&zoom_style, &lv_font_dejavu_16_persian_hebrew);
    //lv_style_set_border_color(&zoom_style, lv_palette_main(LV_PALETTE_GREY));
    //lv_style_set_border_side(&zoom_style, LV_BORDER_SIDE_INTERNAL);
    //lv_style_set_radius(&zoom_style, 0);

    lv_btnmatrix_set_map(zoom_buttons, map);
    lv_btnmatrix_set_btn_ctrl_all(zoom_buttons, LV_BTNMATRIX_CTRL_CLICK_TRIG);
    lv_obj_add_style(zoom_buttons, &zoom_style_bg, 0);
    lv_obj_add_style(zoom_buttons, &zoom_style, LV_PART_ITEMS);
    lv_obj_set_size(zoom_buttons, 80, 50);
    lv_obj_align(zoom_buttons, LV_ALIGN_TOP_RIGHT, 0, 0);
    lv_obj_add_event_cb(zoom_buttons, zoom_btnmatrix_cb, LV_EVENT_VALUE_CHANGED, &graph);

    // function text area setup
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
    //lv_style_set_pad_left(&textarea_style, 3);
    
    lv_style_set_text_align(&textarea_style, LV_ALIGN_LEFT_MID);

    //lv_style_set_text_color(&textarea_style, lv_color_make(255, 0, 0));
    lv_textarea_set_one_line(function_text_area, true);
    lv_obj_add_style(function_text_area, &textarea_style, 0);
    lv_obj_set_size(function_text_area, 180, 30);
    lv_obj_align(function_text_area, LV_ALIGN_TOP_MID, -20, 10);
    lv_obj_add_event_cb(function_text_area, textarea_cb, LV_EVENT_VALUE_CHANGED, &graph);
    
    // function button setup
    lv_style_init(&function_button_style);
    lv_style_set_text_font(&function_button_style, &lv_font_montserrat_12_subpx);
    lv_style_set_text_align(&function_button_style, LV_ALIGN_CENTER);
    lv_style_set_pad_all(&function_button_style, 6);
    //lv_style_set_align(&function_button_style, LV_ALIGN_TOP_MID);
    
    //lv_dropdown_add_option(function_button, "f(x)", 0);
    //lv_dropdown_set_options(function_button, "f(x)\ng(x)\nh(x)");
    lv_dropdown_set_symbol(function_button , NULL);
    lv_obj_add_style(function_button, &function_button_style, 0);
    lv_obj_add_style(function_button, &function_button_style, LV_PART_ITEMS);
    lv_obj_set_size(function_button, 40, 30);
    lv_obj_align(function_button, LV_ALIGN_TOP_MID, 50, 10);
    lv_obj_add_event_cb(function_button, dropdown_button_cb, LV_EVENT_VALUE_CHANGED, &graph);
    // graph setup
    
    graph.set_function_button(function_button);
    graph.set_function_textarea(function_text_area);
    graph.set_scale(mpf_class("0.5"));
    graph.add_function("");
    
    // graph.add_function(plot_sin, LV_COLOR_MAKE16(255, 0, 0), "sin(x)");
    // graph.add_function([](mpf_class x){return x;}, LV_COLOR_MAKE16(0, 255, 0), "x");

    std::cout <<"EVERYTHING ADDED\n";
    //graph.update(); 
	std::cout <<"Graphing updated\n";
}

lv_obj_t* get_functionTextArea()  {
            return function_text_area;
        }
