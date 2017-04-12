#ifndef UI_SPRITE_BASIC_SET_ATTRS_H
#define UI_SPRITE_BASIC_SET_ATTRS_H
#include "ui_sprite_basic_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char * UI_SPRITE_BASIC_SET_ATTRS_NAME;

ui_sprite_basic_set_attrs_t ui_sprite_basic_set_attrs_create(ui_sprite_fsm_state_t fsm_state, const char * name);
void ui_sprite_basic_set_attrs_free(ui_sprite_basic_set_attrs_t send_evt);

const char * ui_sprite_basic_set_attrs_setter(ui_sprite_basic_set_attrs_t set_attrs);
int ui_sprite_basic_set_attrs_set_setter(ui_sprite_basic_set_attrs_t set_attrs, const char * setter);

#ifdef __cplusplus
}
#endif

#endif
