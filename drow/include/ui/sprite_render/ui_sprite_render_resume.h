#ifndef UI_SPRITE_RENDER_RESUME_H
#define UI_SPRITE_RENDER_RESUME_H
#include "ui_sprite_render_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char * UI_SPRITE_RENDER_RESUME_NAME;

ui_sprite_render_resume_t ui_sprite_render_resume_create(ui_sprite_fsm_state_t fsm_state, const char * name);
void ui_sprite_render_resume_free(ui_sprite_render_resume_t send_evt);

#ifdef __cplusplus
}
#endif

#endif
