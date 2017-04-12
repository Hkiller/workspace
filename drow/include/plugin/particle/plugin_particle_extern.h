#ifndef UI_PLUGIN_PARTICLE_EXTERN_H
#define UI_PLUGIN_PARTICLE_EXTERN_H
#include "plugin_particle_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef int (*plugin_particle_extern_init_emitter_fun_t)(
    void * ctx, plugin_particle_obj_emitter_t emitter, const void * data, size_t data_size);

typedef int (*plugin_particle_extern_init_particle_fun_t)(
    void * ctx, plugin_particle_obj_particle_t particle, const void * data, size_t data_size);
    
plugin_particle_extern_t
plugin_particle_extern_create(
    plugin_particle_module_t module, LPDRMETA data_meta,
    void * ctx,
    plugin_particle_extern_init_emitter_fun_t init_emitter,
    plugin_particle_extern_init_particle_fun_t init_particle);

void plugin_particle_extern_free(plugin_particle_extern_t ext);

#ifdef __cplusplus
}
#endif

#endif

