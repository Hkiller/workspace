#ifndef UI_SPRITE_BASIC_DESTORY_SELF_H
#define UI_SPRITE_BASIC_DESTORY_SELF_H
#include "ui_sprite_basic_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char * UI_SPRITE_BASIC_DESTORY_SELF_NAME;

ui_sprite_basic_destory_self_t ui_sprite_basic_destory_self_create(ui_sprite_fsm_state_t fsm_state, const char * name);
void ui_sprite_basic_destory_self_free(ui_sprite_basic_destory_self_t destory_self);

#ifdef __cplusplus
}
#endif

#endif
