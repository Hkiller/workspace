#ifndef UI_SPRITE_SPINE_ACTION_BIND_PARTS_I_H
#define UI_SPRITE_SPINE_ACTION_BIND_PARTS_I_H
#include "plugin/spine/plugin_spine_obj.h"
#include "ui/sprite_2d/ui_sprite_2d_part.h"
#include "ui/sprite_render/ui_sprite_render_types.h"
#include "ui/sprite_spine/ui_sprite_spine_bind_parts.h"
#include "ui_sprite_spine_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum ui_sprite_spine_bind_target {
    ui_sprite_spine_bind_target_skeleton,
    ui_sprite_spine_bind_target_slot,
} ui_sprite_spine_bind_target_t;

struct ui_sprite_spine_bind_parts {
    ui_sprite_spine_module_t m_module;
    char * m_cfg_obj_name;
    char * m_cfg_prefix;
    ui_sprite_spine_bind_target_t m_bind_target;
    uint8_t m_cfg_debug;

    ui_sprite_spine_bind_parts_binding_list_t m_bindings;
};

struct ui_sprite_spine_bind_parts_binding {
    ui_sprite_spine_bind_parts_t m_owner;
    TAILQ_ENTRY(ui_sprite_spine_bind_parts_binding) m_next;
    ui_sprite_2d_part_t m_part;
    ui_sprite_2d_part_attr_t m_attr_enable;    
    struct spBone * m_bone;
    struct spSlot * m_slot;
};

int ui_sprite_spine_bind_parts_regist(ui_sprite_spine_module_t module);
void ui_sprite_spine_bind_parts_unregist(ui_sprite_spine_module_t module);

ui_sprite_spine_bind_parts_binding_t
ui_sprite_spine_bind_parts_binding_create(
    ui_sprite_spine_bind_parts_t bind_parts, ui_sprite_2d_part_t part, struct spBone * bone, struct spSlot * slot);

void ui_sprite_spine_bind_parts_binding_free(ui_sprite_spine_bind_parts_binding_t binding);

void ui_sprite_spine_bind_parts_binding_update(ui_sprite_spine_bind_parts_binding_t binding, ui_transform_t local_transform);
    
void ui_sprite_spine_bind_parts_binding_real_free(ui_sprite_spine_bind_parts_binding_t binding);

#ifdef __cplusplus
}
#endif

#endif
