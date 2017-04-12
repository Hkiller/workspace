#ifndef UI_SPRITE_UI_ENV_H
#define UI_SPRITE_UI_ENV_H
#include "ui_sprite_ui_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char * UI_SPRITE_UI_ENV_NAME;

ui_sprite_ui_env_t ui_sprite_ui_env_create(ui_sprite_ui_module_t module, ui_sprite_world_t world);
void ui_sprite_ui_env_free(ui_sprite_world_t world);

ui_sprite_ui_env_t ui_sprite_ui_env_get(ui_sprite_ui_module_t module);
    
ui_sprite_ui_env_t ui_sprite_ui_env_find(ui_sprite_world_t world);

plugin_ui_env_t ui_sprite_ui_env_env(ui_sprite_ui_env_t env);
ui_sprite_entity_t ui_sprite_ui_env_entity(ui_sprite_ui_env_t env);

void ui_sprite_ui_env_set_debug(ui_sprite_ui_env_t env, uint8_t debug);

int ui_sprite_ui_env_load(ui_sprite_ui_env_t env, cfg_t cfg, const char * language);
int ui_sprite_ui_env_set_language(ui_sprite_ui_env_t env, cfg_t cfg, const char * language);
    
#ifdef __cplusplus
}
#endif

#endif
