#ifndef UI_SPRITE_TRI_MODULE_I_H
#define UI_SPRITE_TRI_MODULE_I_H
#include "cpe/pal/pal_queue.h"
#include "cpe/utils/hash.h"
#include "gd/app/app_types.h"
#include "ui/sprite_cfg/ui_sprite_cfg_loader.h"
#include "ui/sprite_tri/ui_sprite_tri_module.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef TAILQ_HEAD(ui_sprite_tri_rule_list, ui_sprite_tri_rule) ui_sprite_tri_rule_list_t;
typedef TAILQ_HEAD(ui_sprite_tri_condition_list, ui_sprite_tri_condition) ui_sprite_tri_condition_list_t;
typedef TAILQ_HEAD(ui_sprite_tri_condition_meta_list, ui_sprite_tri_condition) ui_sprite_tri_condition_meta_list_t;    
typedef TAILQ_HEAD(ui_sprite_tri_action_list, ui_sprite_tri_action) ui_sprite_tri_action_list_t;
typedef TAILQ_HEAD(ui_sprite_tri_action_meta_list, ui_sprite_tri_action) ui_sprite_tri_action_meta_list_t;    
typedef TAILQ_HEAD(ui_sprite_tri_trigger_list, ui_sprite_tri_trigger) ui_sprite_tri_trigger_list_t;

struct ui_sprite_tri_module {
    gd_app_context_t m_app;
    mem_allocrator_t m_alloc;
    ui_sprite_repository_t m_repo;
    ui_sprite_fsm_module_t m_fsm_module;
    ui_sprite_cfg_loader_t m_loader;
    error_monitor_t m_em;
    int m_debug;

    uint32_t m_condition_count;
    uint32_t m_condition_max_data_capacity;
    struct cpe_hash_table m_condition_metas;

    uint32_t m_action_count;
    uint32_t m_action_max_data_capacity;
    struct cpe_hash_table m_action_metas;
    
    ui_sprite_tri_rule_list_t m_free_rules;
    ui_sprite_tri_condition_list_t m_free_conditions;
    ui_sprite_tri_action_list_t m_free_actions;
    ui_sprite_tri_trigger_list_t m_free_triggers;
};

#ifdef __cplusplus
}
#endif

#endif
