#ifndef PLUGIN_SPINE_OBJ_ANIM_I_H
#define PLUGIN_SPINE_OBJ_ANIM_I_H
#include "plugin/spine/plugin_spine_obj_anim.h"
#include "plugin_spine_obj_track_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_spine_obj_anim {
    plugin_spine_obj_track_t m_track;
    TAILQ_ENTRY(plugin_spine_obj_anim) m_next;
    plugin_spine_obj_anim_group_binding_list_t m_groups;
    spTrackEntry * m_track_entry;
    uint16_t m_loop_count;
    plugin_spine_track_listener_t m_listeners;
};

plugin_spine_obj_anim_t
plugin_spine_obj_anim_create_i(
    plugin_spine_obj_track_t track, spAnimation * anim, uint16_t loop_count);
    
void plugin_spine_obj_anim_real_free_all(plugin_spine_module_t module);

void plugin_spine_obj_anim_dispose(spTrackEntry* entry);
    
#ifdef __cplusplus
}
#endif

#endif
