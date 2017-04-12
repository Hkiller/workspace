#ifndef UI_SPRITE_UI_ACTION_SHOW_TEMPLATE_H
#define UI_SPRITE_UI_ACTION_SHOW_TEMPLATE_H
#include "ui_sprite_ui_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char * UI_SPRITE_UI_ACTION_SHOW_TEMPLATE_TYPE_NAME;

ui_sprite_ui_action_show_template_t ui_sprite_ui_action_show_template_create(ui_sprite_fsm_state_t fsm_state, const char * name);
void ui_sprite_ui_action_show_template_free(ui_sprite_ui_action_show_template_t action_show_template);

const char * ui_sprite_ui_action_show_template_res(ui_sprite_ui_action_show_template_t action_show_template);
void ui_sprite_ui_action_show_template_set_res(ui_sprite_ui_action_show_template_t action_show_template, const char * res);

const char * ui_sprite_ui_action_show_template_group(ui_sprite_ui_action_show_template_t action_show_template);
void ui_sprite_ui_action_show_template_set_group(ui_sprite_ui_action_show_template_t action_show_template, const char * group);

#ifdef __cplusplus
}
#endif

#endif
