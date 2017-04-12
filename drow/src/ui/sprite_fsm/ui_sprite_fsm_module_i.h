#ifndef UI_SPRITE_FSM_MODULE_I_H
#define UI_SPRITE_FSM_MODULE_I_H
#include "cpe/pal/pal_queue.h"
#include "cpe/utils/hash.h"
#include "cpe/utils/buffer.h"
#include "ui/sprite_fsm/ui_sprite_fsm_module.h"
#include "protocol/ui/sprite_fsm/ui_sprite_fsm_data.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef TAILQ_HEAD(ui_sprite_fsm_action_list, ui_sprite_fsm_action) ui_sprite_fsm_action_list_t;
typedef TAILQ_HEAD(ui_sprite_fsm_transition_list, ui_sprite_fsm_transition) ui_sprite_fsm_transition_list_t;
typedef TAILQ_HEAD(ui_sprite_fsm_convertor_list, ui_sprite_fsm_convertor) ui_sprite_fsm_convertor_list_t;

struct ui_sprite_fsm_module {
    gd_app_context_t m_app;
    mem_allocrator_t m_alloc;
    error_monitor_t m_em;

    LPDRMETA m_meta_action_enter_event;

    ui_sprite_repository_t m_repo;
    struct cpe_hash_table m_fsm_action_metas;
    ui_sprite_fsm_action_list_t m_op_actions;
    int m_debug;

    struct mem_buffer m_dump_buffer;
};

#ifdef __cplusplus
}
#endif

#endif
