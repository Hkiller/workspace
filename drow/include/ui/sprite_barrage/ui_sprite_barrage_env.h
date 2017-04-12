#ifndef UI_SPRITE_BARRAGE_ENV_H
#define UI_SPRITE_BARRAGE_ENV_H
#include "ui_sprite_barrage_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char * UI_SPRITE_BARRAGE_ENV_NAME;

ui_sprite_barrage_env_t ui_sprite_barrage_env_create(ui_sprite_barrage_module_t module, ui_sprite_world_t world);
void ui_sprite_barrage_env_free(ui_sprite_world_t world);

ui_sprite_barrage_env_t ui_sprite_barrage_env_find(ui_sprite_world_t world);

plugin_barrage_env_t ui_sprite_barrage_env_env(ui_sprite_barrage_env_t env);

int ui_sprite_barrage_env_set_update_priority(ui_sprite_barrage_env_t env, int8_t priority);

#ifdef __cplusplus
}
#endif

#endif
