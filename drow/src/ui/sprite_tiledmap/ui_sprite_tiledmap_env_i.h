#ifndef UI_SPRITE_TILEDMAP_ENV_I_H
#define UI_SPRITE_TILEDMAP_ENV_I_H
#include "ui/sprite_tiledmap/ui_sprite_tiledmap_env.h"
#include "ui_sprite_tiledmap_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_tiledmap_env {
    ui_sprite_tiledmap_module_t m_module;
    uint8_t m_debug;
    plugin_tiledmap_env_t m_env;
};

int ui_sprite_tiledmap_env_regist(ui_sprite_tiledmap_module_t module);
void ui_sprite_tiledmap_env_unregist(ui_sprite_tiledmap_module_t module);

ui_sprite_world_res_t ui_sprite_tiledmap_env_load(void * ctx, ui_sprite_world_t world, cfg_t cfg);

int ui_sprite_tiledmap_env_create_obj(
    void * ctx, plugin_tiledmap_layer_t layer,
    uint8_t * ignore,
    ui_vector_2_t pos, plugin_tiledmap_data_tile_t tile);
    
#ifdef __cplusplus
}
#endif

#endif
