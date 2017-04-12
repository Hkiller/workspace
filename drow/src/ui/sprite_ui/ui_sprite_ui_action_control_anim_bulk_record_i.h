#ifndef UI_SPRITE_UI_ACTION_CONTROL_ANIM_BULK_RECORD_I_H
#define UI_SPRITE_UI_ACTION_CONTROL_ANIM_BULK_RECORD_I_H
#include "ui_sprite_ui_action_control_anim_bulk_i.h"

#ifdef __cplusplus
extern "C" {
#endif

enum ui_sprite_ui_action_control_anim_bulk_record_state {
    ui_sprite_ui_action_control_anim_bulk_record_init,
    ui_sprite_ui_action_control_anim_bulk_record_runing,
    ui_sprite_ui_action_control_anim_bulk_record_waiting,
};
    
struct ui_sprite_ui_action_control_anim_bulk_record {
    ui_sprite_ui_action_control_anim_bulk_t m_control_anim_bulk;
    TAILQ_ENTRY(ui_sprite_ui_action_control_anim_bulk_record) m_next_for_bulk;
    enum ui_sprite_ui_action_control_anim_bulk_record_state m_state;
    TAILQ_ENTRY(ui_sprite_ui_action_control_anim_bulk_record) m_next_for_state;
    ui_sprite_ui_action_control_anim_bulk_record_t m_parent;
    TAILQ_ENTRY(ui_sprite_ui_action_control_anim_bulk_record) m_next_for_parent;
    ui_sprite_ui_action_control_anim_bulk_record_list_t m_follows;
    float m_cfg_delay_ms;
    uint32_t m_cfg_loop_count;
    float m_cfg_loop_delay_ms;
    char * m_cfg_setup;
    char * m_cfg_teardown;
    char * m_cfg_init;
    char * m_cfg_control;
    char * m_cfg_condition;
    char * m_cfg_anim;
    uint32_t m_animation_id;
    float m_waiting_duration;
};

int ui_sprite_ui_action_control_anim_bulk_record_setup(
    ui_sprite_ui_module_t module, ui_sprite_entity_t entity, ui_sprite_fsm_action_t fsm_action,
    ui_sprite_ui_action_control_anim_bulk_record_t record);

int ui_sprite_ui_action_control_anim_bulk_record_teardown(
    ui_sprite_ui_module_t module, ui_sprite_entity_t entity, ui_sprite_fsm_action_t fsm_action,
    ui_sprite_ui_action_control_anim_bulk_record_t record);
    
int ui_sprite_ui_action_control_anim_bulk_record_enter(
    ui_sprite_ui_module_t module, ui_sprite_entity_t entity, ui_sprite_fsm_action_t fsm_action,
    ui_sprite_ui_action_control_anim_bulk_record_t record);

void ui_sprite_ui_action_control_anim_bulk_record_exit(
    ui_sprite_ui_module_t module, ui_sprite_ui_action_control_anim_bulk_record_t record);

void ui_sprite_ui_action_control_anim_bulk_record_set_state(
    ui_sprite_ui_module_t module,
    ui_sprite_ui_action_control_anim_bulk_record_t record,
    enum ui_sprite_ui_action_control_anim_bulk_record_state state);
    
#ifdef __cplusplus
}
#endif

#endif
