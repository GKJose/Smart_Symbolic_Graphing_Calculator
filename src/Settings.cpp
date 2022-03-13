#include <Settings.hxx>

static lv_obj_t * slider;
static lv_obj_t * slider_label;
static void slider_event_cb(lv_event_t * e)
{

    slider = lv_event_get_target(e);
    char buf[8];
    lv_snprintf(buf, sizeof(buf), "%d%%", (int)lv_slider_get_value(slider));
    lv_label_set_text(slider_label, buf);
    lv_obj_align_to(slider_label, slider, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
    #if ENABLE_PI
    softPwmWrite(5,lv_slider_get_value(slider));
    #endif
	
	
    
}
void createSettingsTab(lv_obj_t* parent){
  /*Create a slider in the center of the display*/
    slider = lv_slider_create(parent);
    lv_obj_center(slider);
    lv_obj_add_event_cb(slider, slider_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

    /*Create a label below the slider*/
    slider_label = lv_label_create(parent);
    lv_label_set_text(slider_label, "0%");

    lv_obj_align_to(slider_label, slider, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
    lv_slider_set_range(slider, 25 , 100);
    #if ENABLE_PI
    softPwmCreate(5,100,100);
    #endif

}

