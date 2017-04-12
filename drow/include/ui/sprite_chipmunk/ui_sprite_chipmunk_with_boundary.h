#ifndef UI_SPRITE_CHIPMUNK_WITH_BOUNDARY_H
#define UI_SPRITE_CHIPMUNK_WITH_BOUNDARY_H
#include "ui_sprite_chipmunk_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char * UI_SPRITE_CHIPMUNK_WITH_BOUNDARY_NAME;

enum ui_sprite_chipmunk_with_boundary_source_rect {
    ui_sprite_chipmunk_with_boundary_source_rect_entity = 1,
    ui_sprite_chipmunk_with_boundary_source_rect_camera = 2,
};
    
ui_sprite_chipmunk_with_boundary_t ui_sprite_chipmunk_with_boundary_create(ui_sprite_fsm_state_t fsm_state, const char * name);
void ui_sprite_chipmunk_with_boundary_free(ui_sprite_chipmunk_with_boundary_t send_evt);

#ifdef __cplusplus
}
#endif

#endif
