#ifndef UI_SPRITE_TRI_ACTION_I_H
#define UI_SPRITE_TRI_ACTION_I_H
#include "ui/sprite_tri/ui_sprite_tri_action.h"
#include "ui_sprite_tri_rule_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_tri_action {
    ui_sprite_tri_rule_t m_rule;
    TAILQ_ENTRY(ui_sprite_tri_action) m_next;
    ui_sprite_tri_action_meta_t m_meta;
    ui_sprite_tri_action_trigger_t m_trigger;
};

void ui_sprite_tri_action_real_free(ui_sprite_tri_action_t action);
void ui_sprite_tri_action_execute(ui_sprite_tri_action_t action);

#ifdef __cplusplus
}
#endif

#endif
