#include <assert.h>
#include "cpe/utils/math_ex.h"
#include "spine/Skeleton.h"
#include "render/utils/ui_quaternion.h"
#include "render/utils/ui_transform.h"
#include "plugin/spine/plugin_spine_utils.h"
#include "render/runtime/ui_runtime_render_obj.h"
#include "ui/sprite_2d/ui_sprite_2d_part_attr.h"
#include "ui/sprite_2d/ui_sprite_2d_part_binding.h"
#include "ui_sprite_spine_follow_parts_i.h"

ui_sprite_spine_follow_parts_binding_t
ui_sprite_spine_follow_parts_binding_create(
    ui_sprite_spine_follow_parts_t follow_parts, ui_sprite_2d_part_t part, plugin_spine_obj_ik_t ik)
{
    ui_sprite_spine_module_t module = follow_parts->m_module;
    ui_sprite_spine_follow_parts_binding_t binding;

    binding = TAILQ_FIRST(&module->m_free_follow_parts_binding);
    if (binding) {
        TAILQ_REMOVE(&module->m_free_follow_parts_binding, binding, m_next);
    }
    else {
        binding = mem_alloc(module->m_alloc, sizeof(struct ui_sprite_spine_follow_parts_binding));
        if (binding == NULL) {
            CPE_ERROR(module->m_em, "ui_sprite_spine_follow_parts_binding_create: alloc fail!");
            return NULL;
        }
    }

    binding->m_owner = follow_parts;
    binding->m_part = part;
    binding->m_ik = ik;

    TAILQ_INSERT_TAIL(&follow_parts->m_bindings, binding, m_next);

    return binding;
}

void ui_sprite_spine_follow_parts_binding_free(ui_sprite_spine_follow_parts_binding_t binding) {
    ui_sprite_spine_follow_parts_t follow_parts = binding->m_owner;
    ui_sprite_spine_module_t module = follow_parts->m_module;

    TAILQ_REMOVE(&follow_parts->m_bindings, binding, m_next);

    binding->m_owner = (void*)module;
    TAILQ_INSERT_TAIL(&module->m_free_follow_parts_binding, binding, m_next);
}

void ui_sprite_spine_follow_parts_binding_real_free(ui_sprite_spine_follow_parts_binding_t binding) {
    ui_sprite_spine_module_t module = (void*)binding->m_owner;

    TAILQ_REMOVE(&module->m_free_follow_parts_binding, binding, m_next);

    mem_free(module->m_alloc, binding);
}

void ui_sprite_spine_follow_parts_binding_update(
    ui_sprite_spine_follow_parts_binding_t binding, ui_transform_t anim_transform_r, ui_vector_2_t pos_adj)
{
    ui_transform part_trans = *ui_sprite_2d_part_trans(binding->m_part);
    ui_vector_2 pos;

    if (pos_adj) {
        ui_transform_get_pos_2(&part_trans, &pos);
        pos.x *= pos_adj->x;
        pos.y *= pos_adj->y;
        ui_transform_set_pos_2(&part_trans, &pos);
    }
    
    if (anim_transform_r) {
        ui_transform_adj_by_parent(&part_trans, anim_transform_r);
    }

    ui_transform_get_pos_2(&part_trans, &pos);

    /* printf("      ik %s set to (%f,%f)\n", ui_sprite_2d_part_name(binding->m_part), pos.x, pos.y); */
    
    plugin_spine_obj_ik_set_target_pos(binding->m_ik, &pos);
}

