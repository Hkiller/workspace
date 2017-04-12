#ifndef UI_SPRITE_TRI_ACTION_META_I_H
#define UI_SPRITE_TRI_ACTION_META_I_H
#include "ui/sprite_tri/ui_sprite_tri_action_meta.h"
#include "ui_sprite_tri_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_tri_action_meta {
    ui_sprite_tri_module_t m_module;
    const char * m_name;
    cpe_hash_entry m_hh;
    size_t m_data_capacity;
    void * m_ctx;
    ui_sprite_tri_action_init_fun_t m_init;
    ui_sprite_tri_action_fini_fun_t m_fini;
    ui_sprite_tri_action_copy_fun_t m_copy;
    ui_sprite_tri_action_exec_fun_t m_exec;
};

void ui_sprite_tri_action_meta_free_all(const ui_sprite_tri_module_t module);
    
uint32_t ui_sprite_tri_action_meta_hash(const ui_sprite_tri_action_meta_t meta);
int ui_sprite_tri_action_meta_eq(const ui_sprite_tri_action_meta_t l, const ui_sprite_tri_action_meta_t r);

#ifdef __cplusplus
}
#endif

#endif
