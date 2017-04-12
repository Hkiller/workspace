#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/string_utils.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui_sprite_spine_chipmunk_with_tri_binding_i.h"

#ifdef __cplusplus
extern "C" {
#endif

ui_sprite_spine_chipmunk_with_tri_binding_t
ui_sprite_spine_chipmunk_with_tri_binding_create(
    ui_sprite_spine_chipmunk_with_tri_t with_tri, ui_sprite_tri_obj_t obj, struct spSlot * slot, struct spAttachment* attachment)
{
    ui_sprite_spine_chipmunk_module_t module = with_tri->m_module;
    ui_sprite_spine_chipmunk_with_tri_binding_t with_tri_binding;

    with_tri_binding = (ui_sprite_spine_chipmunk_with_tri_binding_t)mem_alloc(with_tri->m_module->m_alloc, sizeof(struct ui_sprite_spine_chipmunk_with_tri_binding));
    if (with_tri_binding == NULL) {
        CPE_ERROR(module->m_em, "ui_sprite_spine_chipmunk_with_tri_binding_create: alloc fail!");
        return NULL;
    }

    with_tri_binding->m_with_tri = with_tri;
    with_tri_binding->m_slot = slot;
    with_tri_binding->m_attachment = attachment;
    with_tri_binding->m_rule = ui_sprite_tri_rule_create(obj);

    if (with_tri_binding->m_rule == NULL) {
        CPE_ERROR(module->m_em, "ui_sprite_spine_chipmunk_with_tri_binding_create: create rule fail!");
        mem_free(with_tri->m_module->m_alloc, with_tri_binding);
        return NULL;
    }
    
    TAILQ_INSERT_TAIL(&with_tri->m_bindings, with_tri_binding, m_next);
    
    return with_tri_binding;
}

void ui_sprite_spine_chipmunk_with_tri_binding_update(ui_sprite_spine_chipmunk_with_tri_binding_t with_tri_binding) {
    ui_sprite_tri_rule_set_active(
        with_tri_binding->m_rule,
        with_tri_binding->m_slot->attachment == with_tri_binding->m_attachment ? 1 : 0);
}

void ui_sprite_spine_chipmunk_with_tri_binding_free(ui_sprite_spine_chipmunk_with_tri_binding_t with_tri_binding) {
    ui_sprite_spine_chipmunk_with_tri_t with_tri = with_tri_binding->m_with_tri;

    ui_sprite_tri_rule_free(with_tri_binding->m_rule);
    with_tri_binding->m_rule = NULL;
    
    TAILQ_REMOVE(&with_tri->m_bindings, with_tri_binding, m_next);

    mem_free(with_tri->m_module->m_alloc, with_tri_binding);
}
    
#ifdef __cplusplus
}
#endif

