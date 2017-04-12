#include <assert.h>
#include "spine/Skeleton.h"
#include "render/utils/ui_transform.h"
#include "plugin/spine/plugin_spine_utils.h"
#include "render/runtime/ui_runtime_render_obj.h"
#include "ui/sprite_2d/ui_sprite_2d_part_attr.h"
#include "ui_sprite_spine_bind_parts_i.h"

ui_sprite_spine_bind_parts_binding_t
ui_sprite_spine_bind_parts_binding_create(
    ui_sprite_spine_bind_parts_t bind_parts, ui_sprite_2d_part_t part, struct spBone * bone, struct spSlot * slot)
{
    ui_sprite_spine_module_t module = bind_parts->m_module;
    ui_sprite_spine_bind_parts_binding_t binding;

    binding = TAILQ_FIRST(&module->m_free_bind_parts_binding);
    if (binding) {
        TAILQ_REMOVE(&module->m_free_bind_parts_binding, binding, m_next);
    }
    else {
        binding = mem_alloc(module->m_alloc, sizeof(struct ui_sprite_spine_bind_parts_binding));
        if (binding == NULL) {
            CPE_ERROR(module->m_em, "ui_sprite_spine_bind_parts_binding_create: alloc fail!");
            return NULL;
        }
    }

    binding->m_owner = bind_parts;
    binding->m_part = part;
    binding->m_attr_enable = NULL;
    binding->m_bone = bone;
    binding->m_slot = slot;

    if (slot) {
        binding->m_attr_enable = ui_sprite_2d_part_attr_find(part, "enable");
        if (binding->m_attr_enable == NULL) {
            binding->m_attr_enable = ui_sprite_2d_part_attr_create(part, "enable");
            if (binding->m_attr_enable == NULL) {
                CPE_ERROR(module->m_em, "ui_sprite_spine_bind_parts_binding_create: create attr enable fail!");
                TAILQ_INSERT_TAIL(&module->m_free_bind_parts_binding, binding, m_next);
                return NULL;
            }
        }

        ui_sprite_2d_part_attr_set_value(binding->m_attr_enable, slot->attachment ? "1" : "0");
        ui_sprite_2d_part_dispatch_event(part);
    }
    
    TAILQ_INSERT_TAIL(&bind_parts->m_bindings, binding, m_next);

    return binding;
}

void ui_sprite_spine_bind_parts_binding_free(ui_sprite_spine_bind_parts_binding_t binding) {
    ui_sprite_spine_bind_parts_t bind_parts = binding->m_owner;
    ui_sprite_spine_module_t module = bind_parts->m_module;

    TAILQ_REMOVE(&bind_parts->m_bindings, binding, m_next);

    binding->m_owner = (void*)module;
    TAILQ_INSERT_TAIL(&module->m_free_bind_parts_binding, binding, m_next);
}

void ui_sprite_spine_bind_parts_binding_real_free(ui_sprite_spine_bind_parts_binding_t binding) {
    ui_sprite_spine_module_t module = (void*)binding->m_owner;

    TAILQ_REMOVE(&module->m_free_bind_parts_binding, binding, m_next);

    mem_free(module->m_alloc, binding);
}

void ui_sprite_spine_bind_parts_binding_update(ui_sprite_spine_bind_parts_binding_t binding, ui_transform_t anim_transform) {
    ui_transform bone_transform;

    if (binding->m_slot) {
        if (plugin_spine_slot_calc_transform(binding->m_slot, &bone_transform) != 0) return;
    }
    else {
        if (plugin_spine_bone_calc_transform(binding->m_bone, &bone_transform) != 0) return;
    }

    /* printf( */
    /*     "xxxxx: spine bone %s bind part %s: (%f,%f)\n", */
    /*     binding->m_bone->data->name, ui_sprite_2d_part_name(binding->m_part), */
    /*     bone_transform.m_m4.m14, bone_transform.m_m4.m24); */
    
    if (anim_transform) {
        ui_transform_adj_by_parent(&bone_transform, anim_transform);
    }
    
    ui_sprite_2d_part_set_trans(binding->m_part, &bone_transform);

    if (binding->m_slot) {
        assert(binding->m_attr_enable);
        ui_sprite_2d_part_attr_set_value(binding->m_attr_enable, binding->m_slot->attachment ? "1" : "0");
    }
    
    ui_sprite_2d_part_dispatch_event(binding->m_part);
}
