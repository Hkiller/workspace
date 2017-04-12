#ifndef UI_SPRITE_UI_ACTION_CONTROL_ANIM_I_H
#define UI_SPRITE_UI_ACTION_CONTROL_ANIM_I_H
#include "plugin/ui/plugin_ui_animation.h"
#include "ui/sprite_ui/ui_sprite_ui_action_control_anim.h"
#include "ui_sprite_ui_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_ui_action_control_anim {
    ui_sprite_ui_module_t m_module;
    char * m_cfg_control;
    char * m_cfg_anim;
    char * m_cfg_init;
    float m_cfg_delay_ms;
    uint32_t m_cfg_loop_count;
    float m_cfg_loop_delay_ms;
    
    uint32_t m_animation_id;
};

int ui_sprite_ui_action_control_anim_regist(ui_sprite_ui_module_t module);
void ui_sprite_ui_action_control_anim_unregist(ui_sprite_ui_module_t module);

ui_sprite_fsm_action_t
ui_sprite_ui_action_control_anim_load(void * ctx, ui_sprite_fsm_state_t fsm_state, const char * name, cfg_t cfg);

ui_sprite_fsm_action_t
ui_sprite_ui_action_control_move_inout_load(void * ctx, ui_sprite_fsm_state_t fsm_state, const char * name, cfg_t cfg);

ui_sprite_fsm_action_t
ui_sprite_ui_action_control_alpha_inout_load(void * ctx, ui_sprite_fsm_state_t fsm_state, const char * name, cfg_t cfg);
    
#ifdef __cplusplus
}
#endif

#endif
