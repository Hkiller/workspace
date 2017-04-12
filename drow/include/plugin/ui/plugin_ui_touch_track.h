#ifndef DROW_PLUGIN_UI_TOUCH_TRACK_H
#define DROW_PLUGIN_UI_TOUCH_TRACK_H
#include "gd/app/app_types.h"
#include "plugin_ui_types.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_ui_touch_track_it {
    plugin_ui_touch_track_t (*next)(struct plugin_ui_touch_track_it * it);
    char m_data[64];
};

plugin_ui_touch_track_t plugin_ui_touch_track_create(plugin_ui_env_t env, int32_t track_id);
plugin_ui_touch_track_t plugin_ui_touch_track_find(plugin_ui_env_t env, int32_t track_id);
void plugin_ui_touch_track_free(plugin_ui_touch_track_t track);
void plugin_ui_touch_tracks(plugin_ui_env_t env, plugin_ui_touch_track_it_t track_it);

void plugin_ui_touch_track_notify_down(plugin_ui_touch_track_t track, ui_vector_2_t pt);
void plugin_ui_touch_track_notify_move(plugin_ui_touch_track_t track, ui_vector_2_t pt);
void plugin_ui_touch_track_notify_rise(plugin_ui_touch_track_t track, ui_vector_2_t pt);
    
plugin_ui_control_t plugin_ui_touch_track_catch_control(plugin_ui_touch_track_t track);

plugin_ui_control_t plugin_ui_touch_track_process_control(plugin_ui_touch_track_t track);
void plugin_ui_touch_track_set_process_control(plugin_ui_touch_track_t track, plugin_ui_control_t process_control);
    
ui_vector_2_t plugin_ui_touch_track_down_pt(plugin_ui_touch_track_t track);
ui_vector_2_t plugin_ui_touch_track_last_pt(plugin_ui_touch_track_t track);
ui_vector_2_t plugin_ui_touch_track_cur_pt(plugin_ui_touch_track_t track);

ui_vector_2_t plugin_ui_touch_track_control_down_pt(plugin_ui_touch_track_t track);

uint8_t plugin_ui_touch_track_is_horz_move(plugin_ui_touch_track_t track);
uint8_t plugin_ui_touch_track_is_vert_move(plugin_ui_touch_track_t track);

/*touch_track_it*/
#define plugin_ui_touch_track_it_next(it) ((it)->next ? (it)->next(it) : NULL)

#ifdef __cplusplus
}
#endif

#endif

