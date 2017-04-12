#ifndef UI_SPRITE_TRI_CONDITION_I_H
#define UI_SPRITE_TRI_CONDITION_I_H
#include "ui/sprite_tri/ui_sprite_tri_condition.h"
#include "ui_sprite_tri_rule_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_tri_condition {
    ui_sprite_tri_rule_t m_rule;
    TAILQ_ENTRY(ui_sprite_tri_condition) m_next;
    ui_sprite_tri_condition_meta_t m_meta;
};

void ui_sprite_tri_condition_real_free(ui_sprite_tri_condition_t condition);

uint8_t ui_sprite_tri_condition_check(ui_sprite_tri_condition_t condition, uint8_t * r);
    
#ifdef __cplusplus
}
#endif

#endif
