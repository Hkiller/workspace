#ifndef UI_SPRITE_RENDER_ANIM_I_H
#define UI_SPRITE_RENDER_ANIM_I_H
#include "ui/sprite_render/ui_sprite_render_anim.h"
#include "ui_sprite_render_layer_i.h"
#include "ui_sprite_render_sch_i.h"

struct ui_sprite_render_anim {
    ui_sprite_render_layer_t m_layer;
    TAILQ_ENTRY(ui_sprite_render_anim) m_next_for_layer;
    ui_sprite_render_sch_t m_sch;
    TAILQ_ENTRY(ui_sprite_render_anim) m_next_for_sch;
    ui_sprite_render_group_t m_group;
    TAILQ_ENTRY(ui_sprite_render_anim) m_next_for_group;
    struct cpe_hash_entry m_hh;
    uint32_t m_anim_id;
    char m_anim_name[32];
    ui_transform m_transform;
    float m_priority;
    uint8_t m_sync_transform;
    uint8_t m_auto_remove;
    ui_runtime_render_obj_ref_t m_render_obj_ref;
};

void ui_sprite_render_anim_real_free(ui_sprite_render_anim_t anim);

uint32_t ui_sprite_render_anim_hash(ui_sprite_render_anim_t anim);
int ui_sprite_render_anim_eq(ui_sprite_render_anim_t l, ui_sprite_render_anim_t r);

#endif
