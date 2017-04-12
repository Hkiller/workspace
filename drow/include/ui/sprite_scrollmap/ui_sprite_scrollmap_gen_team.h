#ifndef UI_SPRITE_SCROLLMAP_GEN_TEAM_H
#define UI_SPRITE_SCROLLMAP_GEN_TEAM_H
#include "ui_sprite_scrollmap_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char * UI_SPRITE_SCROLLMAP_GEN_TEAM_NAME;

ui_sprite_scrollmap_gen_team_t ui_sprite_scrollmap_gen_team_create(ui_sprite_fsm_state_t fsm_state, const char * name);
void ui_sprite_scrollmap_gen_team_free(ui_sprite_scrollmap_gen_team_t send_evt);

int ui_sprite_scrollmap_gen_team_set_res(ui_sprite_scrollmap_gen_team_t gen_team, const char * res);
const char * ui_sprite_scrollmap_gen_team_res(ui_sprite_scrollmap_gen_team_t gen_team);

int ui_sprite_scrollmap_gen_team_set_layer(ui_sprite_scrollmap_gen_team_t gen_team, const char * layer);
const char * ui_sprite_scrollmap_gen_team_layer(ui_sprite_scrollmap_gen_team_t gen_team);
    
#ifdef __cplusplus
}
#endif

#endif
