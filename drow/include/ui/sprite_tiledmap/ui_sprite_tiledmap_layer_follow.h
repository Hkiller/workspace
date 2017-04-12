#ifndef UI_SPRITE_TILEDMAP_LAYERFOLLOW_H
#define UI_SPRITE_TILEDMAP_LAYERFOLLOW_H
#include "ui_sprite_tiledmap_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char * UI_SPRITE_TILEDMAP_LAYER_FOLLOW_NAME;

ui_sprite_tiledmap_layer_follow_t ui_sprite_tiledmap_layer_follow_create(ui_sprite_fsm_state_t fsm_state, const char * name);
void ui_sprite_tiledmap_layer_follow_free(ui_sprite_tiledmap_layer_follow_t send_evt);

#ifdef __cplusplus
}
#endif

#endif
