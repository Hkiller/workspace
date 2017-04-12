#ifndef UI_SPRITE_RENDER_ACTION_ADJ_PRIORITY_I_H
#define UI_SPRITE_RENDER_ACTION_ADJ_PRIORITY_I_H
#include "ui/sprite_render/ui_sprite_render_action_adj_priority.h"
#include "ui_sprite_render_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_render_action_adj_priority {
    ui_sprite_render_module_t m_module;
    char * m_cfg_group;
    char * m_cfg_action_adj_priority;
    ui_sprite_render_group_t m_group;
    float m_base_priority;
};

int ui_sprite_render_action_adj_priority_regist(ui_sprite_render_module_t module);
void ui_sprite_render_action_adj_priority_unregist(ui_sprite_render_module_t module);

#ifdef __cplusplus
}
#endif

#endif
