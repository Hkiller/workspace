#ifndef DROW_PLUGIN_UI_ANIM_LABEL_TIME_DURATION_H
#define DROW_PLUGIN_UI_ANIM_LABEL_TIME_DURATION_H
#include "plugin_ui_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char * PLUGIN_UI_ANIM_LABEL_TIME_DURATION;

typedef enum plugin_ui_anim_label_time_duration_formator_category {
    plugin_ui_anim_label_time_duration_formator_done,
    plugin_ui_anim_label_time_duration_formator_sec,
    plugin_ui_anim_label_time_duration_formator_min,
    plugin_ui_anim_label_time_duration_formator_hor,
    plugin_ui_anim_label_time_duration_formator_day,
    plugin_ui_anim_label_time_duration_formator_mon,
    plugin_ui_anim_label_time_duration_formator_year,
} plugin_ui_anim_label_time_duration_formator_category_t;

plugin_ui_anim_label_time_duration_t plugin_ui_anim_label_time_duration_create(plugin_ui_env_t env);
int plugin_ui_anim_label_time_duration_add_frame(plugin_ui_anim_label_time_duration_t anim_resize, plugin_ui_control_frame_t frame);

void plugin_ui_anim_label_time_duration_set_duration(plugin_ui_anim_label_time_duration_t label_time_duration, float duration);
    
void plugin_ui_anim_label_time_duration_set_formator(
    plugin_ui_anim_label_time_duration_t label_time_duration,
    plugin_ui_anim_label_time_duration_formator_category_t category, uint32_t msg_id);    

int plugin_ui_anim_label_time_duration_set_done_res(
    plugin_ui_anim_label_time_duration_t label_time_duration, const char * down_res);
    
#ifdef __cplusplus
}
#endif

#endif
