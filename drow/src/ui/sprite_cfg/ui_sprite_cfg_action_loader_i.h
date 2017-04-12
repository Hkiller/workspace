#ifndef UI_SPRITE_CFG_ACTION_LOADER_I_H
#define UI_SPRITE_CFG_ACTION_LOADER_I_H
#include "ui_sprite_cfg_loader_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_cfg_action_loader {
    ui_sprite_cfg_loader_t m_loader;
    const char * m_name;
    void * m_ctx;
    ui_sprite_cfg_load_action_fun_t m_fun;
    struct cpe_hash_entry m_hh_for_loader;
};
    
ui_sprite_cfg_action_loader_t
ui_sprite_cfg_action_loader_create(
    ui_sprite_cfg_loader_t loader,
    const char * name,
    void * ctx,
    ui_sprite_cfg_load_action_fun_t fun);

ui_sprite_cfg_action_loader_t ui_sprite_cfg_action_loader_find(ui_sprite_cfg_loader_t loader, const char * name);

void ui_sprite_cfg_action_loader_free(ui_sprite_cfg_action_loader_t comp);

void ui_sprite_cfg_action_loader_free_all(ui_sprite_cfg_loader_t loader);

uint32_t ui_sprite_cfg_action_loader_hash(const ui_sprite_cfg_action_loader_t loader);
int ui_sprite_cfg_action_loader_eq(const ui_sprite_cfg_action_loader_t l, const ui_sprite_cfg_action_loader_t r);

#ifdef __cplusplus
}
#endif

#endif
