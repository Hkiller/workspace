#ifndef UI_SPRITE_UI_ACTION_CONTROL_ANIM_BULK_H
#define UI_SPRITE_UI_ACTION_CONTROL_ANIM_BULK_H
#include "ui_sprite_ui_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char * UI_SPRITE_UI_ACTION_CONTROL_ANIM_BULK_NAME;

ui_sprite_ui_action_control_anim_bulk_t ui_sprite_ui_action_control_anim_bulk_create(ui_sprite_fsm_state_t fsm_state, const char * name);
void ui_sprite_ui_action_control_anim_bulk_free(ui_sprite_ui_action_control_anim_bulk_t anim_bulk);

ui_sprite_ui_action_control_anim_bulk_record_t
ui_sprite_ui_action_control_anim_bulk_record_create(
    ui_sprite_ui_action_control_anim_bulk_t anim_bulk,
    float delay_ms, uint32_t loop_count, float loop_delay_ms,
    const char * setup, const char * teardown, const char * init,
    const char * control, const char * condition, const char * res,
    ui_sprite_ui_action_control_anim_bulk_record_t parent);

ui_sprite_ui_action_control_anim_bulk_record_t
ui_sprite_ui_action_control_anim_bulk_record_clone(
    ui_sprite_ui_action_control_anim_bulk_t anim_bulk, 
    ui_sprite_ui_action_control_anim_bulk_record_t parent,
    ui_sprite_ui_action_control_anim_bulk_record_t from);
    
void ui_sprite_ui_action_control_anim_bulk_record_free(ui_sprite_ui_action_control_anim_bulk_record_t record);

void ui_sprite_ui_action_control_anim_bulk_record_free_tree(ui_sprite_ui_action_control_anim_bulk_record_t record);

#ifdef __cplusplus
}
#endif

#endif
