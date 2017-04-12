#ifndef UI_SPRITE_SPINE_ACTION_FOLLOW_PARTS_I_H
#define UI_SPRITE_SPINE_ACTION_FOLLOW_PARTS_I_H
#include "plugin/spine/plugin_spine_obj.h"
#include "plugin/spine/plugin_spine_obj_ik.h"
#include "ui/sprite_2d/ui_sprite_2d_part.h"
#include "ui/sprite_render/ui_sprite_render_types.h"
#include "ui/sprite_spine/ui_sprite_spine_follow_parts.h"
#include "ui_sprite_spine_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_spine_follow_parts {
    ui_sprite_spine_module_t m_module;
    char * m_cfg_obj_name;
    char * m_cfg_prefix;
    char * m_cfg_scope;    
    uint8_t m_cfg_restore;
    
    ui_sprite_spine_follow_parts_binding_list_t m_bindings;
};

struct ui_sprite_spine_follow_parts_binding {
    ui_sprite_spine_follow_parts_t m_owner;
    TAILQ_ENTRY(ui_sprite_spine_follow_parts_binding) m_next;
    ui_sprite_2d_part_t m_part;
    plugin_spine_obj_ik_t m_ik;
};

int ui_sprite_spine_follow_parts_regist(ui_sprite_spine_module_t module);
void ui_sprite_spine_follow_parts_unregist(ui_sprite_spine_module_t module);

ui_sprite_spine_follow_parts_binding_t
ui_sprite_spine_follow_parts_binding_create(
    ui_sprite_spine_follow_parts_t follow_parts, ui_sprite_2d_part_t part, plugin_spine_obj_ik_t ik);

void ui_sprite_spine_follow_parts_binding_free(ui_sprite_spine_follow_parts_binding_t follow);

void ui_sprite_spine_follow_parts_binding_update(
    ui_sprite_spine_follow_parts_binding_t follow, ui_transform_t anim_transform_r, ui_vector_2_t pos_adj);
    
void ui_sprite_spine_follow_parts_binding_real_free(ui_sprite_spine_follow_parts_binding_t follow);

#ifdef __cplusplus
}
#endif

#endif
