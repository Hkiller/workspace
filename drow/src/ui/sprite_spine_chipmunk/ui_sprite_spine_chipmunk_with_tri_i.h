#ifndef UI_SPRITE_PARTICLE_CHIPMUNK_WITH_TRI_I_H
#define UI_SPRITE_PARTICLE_CHIPMUNK_WITH_TRI_I_H
#include "spine/spine.h"
#include "ui/sprite_chipmunk/ui_sprite_chipmunk_env.h"
#include "ui/sprite_spine_chipmunk/ui_sprite_spine_chipmunk_with_tri.h"
#include "ui_sprite_spine_chipmunk_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_spine_chipmunk_with_tri {
    ui_sprite_spine_chipmunk_module_t m_module;
    char * m_cfg_anim_name;
    char * m_cfg_prefix;
    ui_sprite_render_anim_t m_render_anim;
    ui_sprite_spine_chipmunk_with_tri_binding_list_t m_bindings;
};

int ui_sprite_spine_chipmunk_with_tri_regist(ui_sprite_spine_chipmunk_module_t module);
void ui_sprite_spine_chipmunk_with_tri_unregist(ui_sprite_spine_chipmunk_module_t module);

int ui_sprite_spine_chipmunk_with_tri_build_condition(
    ui_sprite_spine_chipmunk_module_t module, ui_sprite_entity_t entity, plugin_spine_obj_t spine_obj,
    ui_sprite_spine_chipmunk_with_tri_t with_tri, struct spSlot * slot,
    ui_sprite_tri_rule_t rule, char * cfg);

int ui_sprite_spine_chipmunk_with_tri_build_action(
    ui_sprite_spine_chipmunk_module_t module, ui_sprite_entity_t entity, plugin_spine_obj_t spine_obj,
    ui_sprite_spine_chipmunk_with_tri_t with_tri, struct spSlot * slot,
    ui_sprite_tri_rule_t rule, ui_sprite_tri_action_trigger_t trigger, char * cfg);

int ui_sprite_spine_chipmunk_with_tri_build_trigger_event(
    ui_sprite_spine_chipmunk_module_t module, ui_sprite_entity_t entity, ui_sprite_tri_rule_t rule, char * cfg);

int ui_sprite_spine_chipmunk_with_tri_build_trigger_attr(
    ui_sprite_spine_chipmunk_module_t module, ui_sprite_entity_t entity, ui_sprite_tri_rule_t rule, char * cfg);
    
#ifdef __cplusplus
}
#endif

#endif
