#ifndef UI_SPRITE_COMPONENT_META_I_H
#define UI_SPRITE_COMPONENT_META_I_H
#include "ui/sprite/ui_sprite_component_meta.h"
#include "ui_sprite_repository_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_component_meta {
    ui_sprite_repository_t m_repo;
    struct cpe_hash_entry m_hh_for_repo;
    ui_sprite_component_list_t m_components;
    const char * m_name;
    LPDRMETA m_data_meta;
    uint16_t m_data_start; 
    uint16_t m_data_size;
    uint16_t m_size;
    void * m_enter_fun_ctx;
    ui_sprite_component_enter_fun_t m_enter_fun;
    void * m_exit_fun_ctx;
    ui_sprite_component_exit_fun_t m_exit_fun;
    void * m_init_fun_ctx;
    ui_sprite_component_init_fun_t m_init_fun;
    void * m_copy_fun_ctx;
    ui_sprite_component_copy_fun_t m_copy_fun;
    void * m_free_fun_ctx;
    ui_sprite_component_free_fun_t m_free_fun;
    void * m_update_fun_ctx;
    ui_sprite_component_update_fun_t m_update_fun;
};

void ui_sprite_component_meta_free_all(ui_sprite_repository_t repo);

uint32_t ui_sprite_component_meta_hash(const ui_sprite_component_meta_t meta);
int ui_sprite_component_meta_eq(const ui_sprite_component_meta_t l, const ui_sprite_component_meta_t r);

#ifdef __cplusplus
}
#endif

#endif
