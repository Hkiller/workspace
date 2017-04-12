#ifndef UI_SPRITE_SCROLLMAP_ENV_H
#define UI_SPRITE_SCROLLMAP_ENV_H
#include "ui_sprite_scrollmap_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char * UI_SPRITE_SCROLLMAP_ENV_NAME;

ui_sprite_scrollmap_env_t ui_sprite_scrollmap_env_create(
    ui_sprite_scrollmap_module_t module, ui_sprite_world_t world,
    plugin_scrollmap_moving_way_t moving_way, ui_vector_2_t base_size);
void ui_sprite_scrollmap_env_free(ui_sprite_world_t world);

ui_sprite_scrollmap_env_t ui_sprite_scrollmap_env_find(ui_sprite_world_t world);

plugin_scrollmap_env_t ui_sprite_scrollmap_env_env(ui_sprite_scrollmap_env_t env);

int ui_sprite_scrollmap_env_set_update_priority(ui_sprite_scrollmap_env_t env, int8_t priority);

ui_sprite_scrollmap_runtime_size_policy_t ui_sprite_scrollmap_env_runtime_size_policy(ui_sprite_scrollmap_env_t env);
void ui_sprite_scrollmap_env_set_runtime_size_policy(ui_sprite_scrollmap_env_t env, ui_sprite_scrollmap_runtime_size_policy_t policy);

#ifdef __cplusplus
}
#endif

#endif
