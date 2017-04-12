#ifndef CPE_FSM_INTERNAL_TYPES_H
#define CPE_FSM_INTERNAL_TYPES_H
#include "cpe/utils/hash.h"
#include "cpe/utils/hash_string.h"
#include "cpe/fsm/fsm_types.h"

struct fsm_def_machine {
    mem_allocrator_t m_alloc;
    error_monitor_t m_em;

    fsm_def_state_t m_init_state;
    fsm_evt_dumper_t m_dumper;

    uint32_t m_state_capacity;
    uint32_t m_state_max;
    fsm_def_state_t * m_states;
    struct cpe_hash_table m_states_by_name;
};

struct fsm_def_state {
    fsm_def_machine_t m_machine;
    uint32_t m_id;
    cpe_hash_string_t m_name;

    fsm_machine_action_t m_enter;
    fsm_machine_action_t m_leave;

    uint32_t m_trans_count;
    fsm_def_transition_t m_base_trans[10];

    uint32_t m_ext_trans_capacity;
    fsm_def_transition_t * m_ext_trans;

    struct cpe_hash_entry m_hh;
};

struct fsm_machine_monitor_node {
    fsm_machine_monitor_t m_process;
    void * m_process_ctx;
    struct fsm_machine_monitor_node * m_next;
};

#ifdef __cplusplus
}
#endif

#endif
