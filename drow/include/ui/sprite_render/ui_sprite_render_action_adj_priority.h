#ifndef UI_SPRITE_RENDER_ACTION_ACTION_ADJ_PRIORITY_H
#define UI_SPRITE_RENDER_ACTION_ACTION_ADJ_PRIORITY_H
#include "ui_sprite_render_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char * UI_SPRITE_RENDER_ACTION_ADJ_PRIORITY_TYPE_NAME;

ui_sprite_render_action_adj_priority_t ui_sprite_render_action_adj_priority_create(ui_sprite_fsm_state_t fsm_state, const char * name);
void ui_sprite_render_action_adj_priority_free(ui_sprite_render_action_adj_priority_t action_adj_priority);

const char * ui_sprite_render_action_adj_priority_group(ui_sprite_render_action_adj_priority_t action_adj_priority);
void ui_sprite_render_action_adj_priority_set_group(ui_sprite_render_action_adj_priority_t action_adj_priority, const char * group);

const char * ui_sprite_render_action_adj_priority_action_adj_priority(ui_sprite_render_action_adj_priority_t action_adj_priority);
void ui_sprite_render_action_adj_priority_set_action_adj_priority(ui_sprite_render_action_adj_priority_t action_adj_priority, const char * name);

#ifdef __cplusplus
}
#endif

#endif
