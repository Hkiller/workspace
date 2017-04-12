#ifndef UI_SPRITE_RENDER_OBJ_CREATOR_I_H
#define UI_SPRITE_RENDER_OBJ_CREATOR_I_H
#include "ui_sprite_render_module_i.h"

typedef struct ui_sprite_render_obj_creator * ui_sprite_render_obj_creator_t;

struct ui_sprite_render_obj_creator {
    ui_sprite_render_module_t m_module;
    struct cpe_hash_entry m_hh;
    const char * m_name;
    ui_sprite_render_obj_create_fun_t m_fun;
    void * m_ctx;
};

ui_sprite_render_obj_creator_t
ui_sprite_render_obj_creator_find(ui_sprite_render_module_t module, const char * name);

void ui_sprite_render_obj_creator_free_all(ui_sprite_render_module_t module);

uint32_t ui_sprite_render_obj_creator_hash(ui_sprite_render_obj_creator_t creator);
int ui_sprite_render_obj_creator_eq(ui_sprite_render_obj_creator_t l, ui_sprite_render_obj_creator_t r);

#endif
