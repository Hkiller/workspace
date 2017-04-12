#ifndef UI_SPRITE_TOUCH_ENV_H
#define UI_SPRITE_TOUCH_ENV_H
#include "render/utils/ui_vector_2.h"
#include "ui_sprite_touch_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char * UI_SPRITE_TOUCH_ENV_NAME;

ui_sprite_touch_env_t ui_sprite_touch_env_create(ui_sprite_touch_mgr_t mgr, ui_sprite_world_t world);
void ui_sprite_touch_env_free(ui_sprite_world_t world);

ui_sprite_touch_env_t ui_sprite_touch_env_find(ui_sprite_world_t world);

#ifdef __cplusplus
}
#endif

#endif
