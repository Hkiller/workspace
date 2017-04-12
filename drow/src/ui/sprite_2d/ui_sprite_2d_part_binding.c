#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "ui_sprite_2d_part_binding_i.h"

ui_sprite_2d_part_binding_t
ui_sprite_2d_part_binding_create(
    ui_sprite_2d_part_t part,
    void * ctx,
    ui_sprite_2d_part_on_updated_fun_t on_updated)
{
    ui_sprite_2d_module_t module = part->m_transform->m_module;
    ui_sprite_2d_part_binding_t part_binding;

    part_binding = TAILQ_FIRST(&module->m_free_part_bindings);
    if (part_binding == NULL) {
        part_binding = mem_alloc(module->m_alloc, sizeof(struct ui_sprite_2d_part_binding));
        if (part_binding == NULL) {
            CPE_ERROR(module->m_em, "ui_sprite_2d_part_binding_create: alloc fail!");
            return NULL;
        }
    }
    else {
        TAILQ_REMOVE(&module->m_free_part_bindings, part_binding, m_next);
    }

    part_binding->m_part = part;
    part_binding->m_ctx = ctx;
    part_binding->m_on_updated = on_updated;

    TAILQ_INSERT_TAIL(&part->m_bindings, part_binding, m_next);
    
    return part_binding;
}

void ui_sprite_2d_part_binding_free(ui_sprite_2d_part_binding_t part_binding) {
    ui_sprite_2d_part_t part = part_binding->m_part;
    ui_sprite_2d_module_t module = part->m_transform->m_module;
    
    TAILQ_REMOVE(&part->m_bindings, part_binding, m_next);
    TAILQ_INSERT_TAIL(&module->m_free_part_bindings, part_binding, m_next);
}

void ui_sprite_2d_part_binding_real_free(ui_sprite_2d_module_t module, ui_sprite_2d_part_binding_t part_binding) {
    TAILQ_REMOVE(&module->m_free_part_bindings, part_binding, m_next);
    mem_free(module->m_alloc, part_binding);
}

void ui_sprite_2d_part_remove_bindings(ui_sprite_2d_part_t part, void * ctx) {
    ui_sprite_2d_part_binding_t part_binding, next;

    for(part_binding = TAILQ_FIRST(&part->m_bindings); part_binding; part_binding = next) {
        next = TAILQ_NEXT(part_binding, m_next);

        if (part_binding->m_ctx == ctx) ui_sprite_2d_part_binding_free(part_binding);
    }
}

void ui_sprite_2d_transform_remove_bindings(ui_sprite_2d_transform_t transform, void * ctx) {
    ui_sprite_2d_part_t part;

    TAILQ_FOREACH(part, &transform->m_parts, m_next) {
        ui_sprite_2d_part_remove_bindings(part, ctx);
    }
}
