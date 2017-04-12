#ifndef PLUGIN_UI_TOUCH_TRACK_I_H
#define PLUGIN_UI_TOUCH_TRACK_I_H
#include "plugin/ui/plugin_ui_touch_track.h"
#include "plugin_ui_env_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_ui_touch_track {
    plugin_ui_env_t m_env;
    TAILQ_ENTRY(plugin_ui_touch_track) m_next_for_env;
    int32_t m_track_id;
    float m_cache_duration;
    uint8_t m_long_push_sended;
    plugin_ui_control_t m_catch_control;
    plugin_ui_control_t m_process_control;    
    ui_vector_2 m_down_pt;
    ui_vector_2 m_last_pt;
    ui_vector_2 m_cur_pt;
    ui_vector_2 m_control_down_pt;
};

void plugin_ui_touch_track_real_free(plugin_ui_touch_track_t track);    

#ifdef __cplusplus
}
#endif

#endif
