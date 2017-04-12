#ifndef UI_SPRITE_TRI_ACTION_REMOVE_SELF_H
#define UI_SPRITE_TRI_ACTION_REMOVE_SELF_H
#include "ui_sprite_tri_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char * UI_SPRITE_TRI_ACTION_REMOVE_SELF;

ui_sprite_tri_action_remove_self_t ui_sprite_tri_action_remove_self_create(ui_sprite_tri_rule_t rule);
void ui_sprite_tri_action_remove_self_free(ui_sprite_tri_action_remove_self_t remove_self);

#ifdef __cplusplus
}
#endif

#endif
