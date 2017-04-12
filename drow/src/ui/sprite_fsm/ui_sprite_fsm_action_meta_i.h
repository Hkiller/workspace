#ifndef UI_SPRITE_FSM_ACTION_META_I_H
#define UI_SPRITE_FSM_ACTION_META_I_H
#include "ui/sprite_fsm/ui_sprite_fsm_action_meta.h"
#include "ui_sprite_fsm_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_fsm_action_meta {
    ui_sprite_fsm_module_t m_module;
    struct cpe_hash_entry m_hh_for_module;
    ui_sprite_fsm_action_list_t m_fsm_actions;
    const char * m_name;
    LPDRMETA m_data_meta;
    uint16_t m_data_start;
    uint16_t m_data_size;
    uint16_t m_size;
    void * m_enter_fun_ctx;
    ui_sprite_fsm_action_enter_fun_t m_enter_fun;
    void * m_exit_fun_ctx;
    ui_sprite_fsm_action_exit_fun_t m_exit_fun;
    void * m_init_fun_ctx;
    ui_sprite_fsm_action_init_fun_t m_init_fun;
    void * m_copy_fun_ctx;
    ui_sprite_fsm_action_copy_fun_t m_copy_fun;
    void * m_free_fun_ctx;
    ui_sprite_fsm_action_free_fun_t m_free_fun;
    void * m_update_fun_ctx;
    ui_sprite_fsm_action_update_fun_t m_update_fun;
};

void ui_sprite_fsm_action_meta_free_all(ui_sprite_fsm_module_t module);

uint32_t ui_sprite_fsm_action_meta_hash(const ui_sprite_fsm_action_meta_t meta);
int ui_sprite_fsm_action_meta_eq(const ui_sprite_fsm_action_meta_t l, const ui_sprite_fsm_action_meta_t r);

#ifdef __cplusplus
}
#endif

#endif
