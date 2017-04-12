#ifndef PLUGIN_TILEDMAP_ENV_H
#define PLUGIN_TILEDMAP_ENV_H
#include "plugin_tiledmap_types.h"

#ifdef __cplusplus
extern "C" {
#endif

plugin_tiledmap_env_t plugin_tiledmap_env_create(plugin_tiledmap_module_t module);
void plugin_tiledmap_env_free(plugin_tiledmap_env_t env);


/*extern obj factory*/
typedef int (*plugin_tiledmap_env_extern_obj_create_fun_t)(
    void * ctx, plugin_tiledmap_layer_t layer,
    uint8_t * ignore,
    ui_vector_2_t pos, plugin_tiledmap_data_tile_t tile);

void plugin_tiledmap_env_set_extern_obj_factory(
    plugin_tiledmap_env_t env,
    void * create_ctx,
    plugin_tiledmap_env_extern_obj_create_fun_t create_fun);
    
#ifdef __cplusplus
}
#endif

#endif
