#ifndef PLUGIN_SPINE_OBJ_TRACK_I_H
#define PLUGIN_SPINE_OBJ_TRACK_I_H
#include "plugin/spine/plugin_spine_obj_track.h"
#include "plugin_spine_obj_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_spine_obj_track {
    plugin_spine_obj_t m_obj;
    TAILQ_ENTRY(plugin_spine_obj_track) m_next;
    char m_name[64];
    uint16_t m_track_index;
    float m_time_scale;
    plugin_spine_obj_anim_list_t m_anims;
    plugin_spine_obj_anim_list_t m_done_anims;
};

void plugin_spine_obj_track_apply_all_animations(plugin_spine_obj_track_t track);
    
void plugin_spine_obj_track_real_free_all(plugin_spine_module_t module);

#ifdef __cplusplus
}
#endif

#endif
