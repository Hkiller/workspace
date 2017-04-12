#ifndef UI_SPRITE_RENDER_SCH_I_H
#define UI_SPRITE_RENDER_SCH_I_H
#include "ui/sprite_render/ui_sprite_render_sch.h"
#include "ui_sprite_render_env_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_render_sch {
    ui_sprite_render_module_t m_module;
    ui_sprite_render_env_t m_render_env;
    ui_sprite_render_anim_list_t m_anims;
    ui_sprite_render_def_list_t m_defs;
    ui_sprite_render_group_list_t m_groups;
    float m_render_priority;
    uint8_t m_is_dirty;
    ui_sprite_render_layer_t m_default_layer;
};

int ui_sprite_render_sch_regist(ui_sprite_render_module_t module);
void ui_sprite_render_sch_unregist(ui_sprite_render_module_t module);
int ui_sprite_render_sch_load(void * ctx, ui_sprite_component_t component, cfg_t cfg);

uint32_t ui_sprite_render_sch_entity_id(ui_sprite_render_sch_t render_sch);
void ui_sprite_render_sch_sort_anims(ui_sprite_render_sch_t sch);
    
#ifdef __cplusplus
}
#endif

#endif
