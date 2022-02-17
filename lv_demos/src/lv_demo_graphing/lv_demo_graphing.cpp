#include "lv_demo_graphing.hxx"
#include "graph.hxx"
#include <math.h>

static mpf_class plot_sin(mpf_class x) {
    return mpf_class(sin(x.get_d()*0.1) * 50.0);
}

static void btn_cb(lv_event_t* event){
    if (!event)
        return;
    switch (event->code){
        case LV_EVENT_PRESSED:
            std::cout << "BUTTON PRESSED\n";
        default:
            break;
    }
}

void create_graph(void){
    static graphing::Graph graph(lv_scr_act());
    graph.add_function(plot_sin, LV_COLOR_MAKE16(255, 0, 0));
    graph.add_function([](mpf_class x){return x;}, LV_COLOR_MAKE16(0, 255, 0));
    graph.update(); 
}


