#ifndef UI_SPRITE_RENDER_LAYER_I_H
#define UI_SPRITE_RENDER_LAYER_I_H
#include "ui/sprite_render/ui_sprite_render_layer.h"
#include "ui_sprite_render_env_i.h"

struct ui_sprite_render_layer {
    ui_sprite_render_env_t m_env;
    TAILQ_ENTRY(ui_sprite_render_layer) m_next;
    char m_name[32];
    uint8_t m_is_dirty;
    uint8_t m_is_free;
    ui_sprite_render_anim_list_t m_anims;
};

void ui_sprite_render_layer_sort_anims(ui_sprite_render_layer_t layer);

#endif
