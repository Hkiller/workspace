#ifndef UI_SPRITE_RENDER_SUSPEND_H
#define UI_SPRITE_RENDER_SUSPEND_H
#include "ui_sprite_render_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char * UI_SPRITE_RENDER_SUSPEND_NAME;

ui_sprite_render_suspend_t ui_sprite_render_suspend_create(ui_sprite_fsm_state_t fsm_state, const char * name);
void ui_sprite_render_suspend_free(ui_sprite_render_suspend_t send_evt);

#ifdef __cplusplus
}
#endif

#endif