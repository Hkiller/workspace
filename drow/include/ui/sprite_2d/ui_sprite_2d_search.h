#ifndef UI_SPRITE_2D_SEARCH_H
#define UI_SPRITE_2D_SEARCH_H
#include "ui_sprite_2d_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char * UI_SPRITE_2D_SEARCH_NAME;

ui_sprite_2d_search_t ui_sprite_2d_search_create(ui_sprite_fsm_state_t fsm_state, const char * name);
void ui_sprite_2d_search_free(ui_sprite_2d_search_t search);

int ui_sprite_2d_search_set_on_found(ui_sprite_2d_search_t search, const char * on_found);
const char * ui_sprite_2d_search_on_found(ui_sprite_2d_search_t search);

#ifdef __cplusplus
}
#endif

#endif
