#ifndef UI_SPRITE_BASIC_JOIN_GROUP_H
#define UI_SPRITE_BASIC_JOIN_GROUP_H
#include "ui_sprite_basic_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char * UI_SPRITE_BASIC_JOIN_GROUP_NAME;

ui_sprite_basic_join_group_t ui_sprite_basic_join_group_create(ui_sprite_fsm_state_t fsm_state, const char * name);
void ui_sprite_basic_join_group_free(ui_sprite_basic_join_group_t send_evt);

const char * ui_sprite_basic_join_group_name(ui_sprite_basic_join_group_t join_group);
int ui_sprite_basic_join_group_set_name(ui_sprite_basic_join_group_t join_group, const char * name);

#ifdef __cplusplus
}
#endif

#endif
