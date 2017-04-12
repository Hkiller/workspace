#ifndef UI_PLUGIN_PARTICLE_OBJ_PLUGIN_I_H
#define UI_PLUGIN_PARTICLE_OBJ_PLUGIN_I_H
#include "plugin/particle/plugin_particle_obj_plugin.h"
#include "plugin_particle_obj_emitter_i.h"

#ifdef __cplusplus
extern "C" {
#endif
        
struct plugin_particle_obj_plugin {
    plugin_particle_obj_emitter_t m_emitter;
    TAILQ_ENTRY(plugin_particle_obj_plugin) m_next;
    plugin_particle_obj_plugin_data_list_t m_datas;
    plugin_particle_obj_plugin_data_list_t m_free_datas;
    void * m_ctx;
    uint32_t m_data_capacity;
    plugin_particle_obj_plugin_init_fun_t m_init_fun;
    plugin_particle_obj_plugin_fini_fun_t m_fini_fun;
    plugin_particle_obj_plugin_update_fun_t m_update_fun;
};

void plugin_particle_obj_plugin_real_free(
    plugin_particle_module_t module, plugin_particle_obj_plugin_t plugin);
    
#ifdef __cplusplus
}
#endif

#endif
