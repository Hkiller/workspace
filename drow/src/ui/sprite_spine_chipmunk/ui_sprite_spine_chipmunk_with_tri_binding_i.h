#ifndef UI_SPRITE_SPINE_CHIPMUNK_WITH_TRI_BINDING_I_H
#define UI_SPRITE_SPINE_CHIPMUNK_WITH_TRI_BINDING_I_H
#include "ui/sprite_tri/ui_sprite_tri_rule.h"
#include "ui_sprite_spine_chipmunk_with_tri_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_spine_chipmunk_with_tri_binding {
    ui_sprite_spine_chipmunk_with_tri_t m_with_tri;
    TAILQ_ENTRY(ui_sprite_spine_chipmunk_with_tri_binding) m_next;
    ui_sprite_tri_rule_t m_rule;
    struct spSlot * m_slot;
    struct spAttachment* m_attachment;
};

ui_sprite_spine_chipmunk_with_tri_binding_t
ui_sprite_spine_chipmunk_with_tri_binding_create(
    ui_sprite_spine_chipmunk_with_tri_t with_tri, ui_sprite_tri_obj_t obj, struct spSlot * slot, struct spAttachment* attachment);
void ui_sprite_spine_chipmunk_with_tri_binding_free(ui_sprite_spine_chipmunk_with_tri_binding_t binding);

void ui_sprite_spine_chipmunk_with_tri_binding_update(ui_sprite_spine_chipmunk_with_tri_binding_t with_tri_binding);

#ifdef __cplusplus
}
#endif

#endif
