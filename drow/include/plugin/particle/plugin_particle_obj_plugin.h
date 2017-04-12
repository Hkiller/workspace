#ifndef UI_PLUGIN_PARTICLE_OBJ_PLUGIN_H
#define UI_PLUGIN_PARTICLE_OBJ_PLUGIN_H
#include "plugin_particle_types.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_particle_obj_plugin_it {
    plugin_particle_obj_plugin_t (*next)(struct plugin_particle_obj_plugin_it * it);
    char m_data[64];
};
    
typedef int (*plugin_particle_obj_plugin_init_fun_t)(void * ctx, plugin_particle_obj_plugin_data_t data);
typedef void (*plugin_particle_obj_plugin_fini_fun_t)(void * ctx, plugin_particle_obj_plugin_data_t data);
typedef void (*plugin_particle_obj_plugin_update_fun_t)(void * ctx, plugin_particle_obj_plugin_data_t data);    
    
plugin_particle_obj_plugin_t
plugin_particle_obj_plugin_create(
    plugin_particle_obj_emitter_t emitter,
    void * ctx,
    uint32_t data_capacity,
    plugin_particle_obj_plugin_init_fun_t init_fun,
    plugin_particle_obj_plugin_fini_fun_t fini_fun,
    plugin_particle_obj_plugin_update_fun_t update_fun);

void plugin_particle_obj_plugin_free(plugin_particle_obj_plugin_t plugin);

plugin_particle_obj_emitter_t plugin_particle_obj_plugin_emitter(plugin_particle_obj_plugin_t plugin);

plugin_particle_obj_plugin_t
plugin_particle_obj_plugin_find_by_ctx(plugin_particle_obj_emitter_t emitter, void * ctx);
    
void * plugin_particle_obj_plugin_ctx(plugin_particle_obj_plugin_t plugin);

void plugin_particle_obj_plugins(plugin_particle_obj_plugin_it_t it, plugin_particle_obj_emitter_t emitter);
    
#define plugin_particle_obj_plugin_it_next(it) ((it)->next ? (it)->next(it) : NULL)

#ifdef __cplusplus
}
#endif

#endif

