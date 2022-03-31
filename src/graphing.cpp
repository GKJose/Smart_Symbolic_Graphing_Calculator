#include <graphing.hxx>
#include <graph.hxx>

void create_graph(lv_obj_t *parent){
    static graphing::Graph graph(parent);
    graph.set_scale(0.5);
    graph.update(); 
}

