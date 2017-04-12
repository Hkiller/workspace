#ifndef UI_SPRITE_TRI_RULE_I_H
#define UI_SPRITE_TRI_RULE_I_H
#include "ui/sprite_tri/ui_sprite_tri_rule.h"
#include "ui_sprite_tri_obj_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_tri_rule {
    ui_sprite_tri_obj_t m_obj;
    TAILQ_ENTRY(ui_sprite_tri_rule) m_next;
    uint8_t m_active;
    uint8_t m_effect;
    ui_sprite_tri_trigger_list_t m_triggers;
    ui_sprite_tri_condition_t m_condition;
    ui_sprite_tri_action_list_t m_actions;
};

void ui_sprite_tri_rule_real_free(ui_sprite_tri_rule_t rule);

void ui_sprite_tri_rule_sync_state(ui_sprite_tri_rule_t rule, uint8_t is_active);
    
#ifdef __cplusplus
}
#endif

#endif
