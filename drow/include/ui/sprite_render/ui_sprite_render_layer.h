#ifndef UI_SPRITE_RENDER_LAYER_H
#define UI_SPRITE_RENDER_LAYER_H
#include "ui_sprite_render_types.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_render_layer_it {
    ui_sprite_render_layer_t (*next)(struct ui_sprite_render_layer_it * it);
    char m_data[64];
};
    
ui_sprite_render_layer_t ui_sprite_render_layer_create(ui_sprite_render_env_t env, ui_sprite_render_layer_t before, const char * name);
void ui_sprite_render_layer_free(ui_sprite_render_layer_t layer);

ui_sprite_render_layer_t ui_sprite_render_layer_default(ui_sprite_render_env_t env);    
ui_sprite_render_layer_t ui_sprite_render_layer_find(ui_sprite_render_env_t env, const char * layer_name);

const char * ui_sprite_render_layer_name(ui_sprite_render_layer_t layer);

void ui_sprite_render_layer_anims(ui_sprite_render_layer_t layer, ui_sprite_render_anim_it_t anim_it);
    
#define ui_sprite_render_layer_it_next(it) ((it)->next ? (it)->next(it) : NULL)
    
#ifdef __cplusplus
}
#endif

#endif
