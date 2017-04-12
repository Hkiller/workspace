#ifndef UI_SPRITE_BASIC_GUARD_ATTRS_H
#define UI_SPRITE_BASIC_GUARD_ATTRS_H
#include "ui_sprite_basic_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char * UI_SPRITE_BASIC_GUARD_ATTRS_NAME;

ui_sprite_basic_guard_attrs_t ui_sprite_basic_guard_attrs_create(ui_sprite_fsm_state_t fsm_state, const char * name);
void ui_sprite_basic_guard_attrs_free(ui_sprite_basic_guard_attrs_t send_evt);

const char * ui_sprite_basic_guard_attrs_set_on_enter(ui_sprite_basic_guard_attrs_t guard_attrs);
int ui_sprite_basic_guard_attrs_guard_on_enter(ui_sprite_basic_guard_attrs_t guard_attrs, const char * on_enter);

const char * ui_sprite_basic_guard_attrs_set_on_exit(ui_sprite_basic_guard_attrs_t guard_attrs);
int ui_sprite_basic_guard_attrs_guard_on_exit(ui_sprite_basic_guard_attrs_t guard_attrs, const char * on_exit);
    
#ifdef __cplusplus
}
#endif

#endif
