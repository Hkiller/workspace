#ifndef UI_SPRITE_TILEDMAP_ENV_H
#define UI_SPRITE_TILEDMAP_ENV_H
#include "ui_sprite_tiledmap_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char * UI_SPRITE_TILEDMAP_ENV_NAME;

ui_sprite_tiledmap_env_t ui_sprite_tiledmap_env_create(ui_sprite_tiledmap_module_t module, ui_sprite_world_t world);
void ui_sprite_tiledmap_env_free(ui_sprite_world_t world);

ui_sprite_tiledmap_env_t ui_sprite_tiledmap_env_find(ui_sprite_world_t world);

plugin_tiledmap_env_t ui_sprite_tiledmap_env_env(ui_sprite_tiledmap_env_t env);
    
#ifdef __cplusplus
}
#endif

#endif
