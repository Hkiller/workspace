#ifndef PLUGIN_SPINE_TRACK_LISTENER_I_H
#define PLUGIN_SPINE_TRACK_LISTENER_I_H
#include "plugin_spine_obj_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_spine_track_listener {
    plugin_spine_anim_event_fun_t m_func;
    void * m_func_ctx;
    plugin_spine_track_listener_t m_next;
};

plugin_spine_track_listener_t plugin_spine_track_listener_create(plugin_spine_module_t module, plugin_spine_anim_event_fun_t fun, void * ctx);
void plugin_spine_track_listener_free_list(plugin_spine_module_t module, plugin_spine_track_listener_t * list);
void plugin_spine_track_listener_real_free_all(plugin_spine_module_t module);
void plugin_spine_track_listener_free_list_by_ctx(plugin_spine_module_t module, plugin_spine_track_listener_t * list, void * ctx);
    
#ifdef __cplusplus
}
#endif

#endif
