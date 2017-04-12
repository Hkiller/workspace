#include <assert.h>
#include "cpe/pal/pal_platform.h"
#include "cpe/pal/pal_string.h"
#include "cpe/fsm/fsm_def.h"
#include "fsm_internal_ops.h"

fsm_def_machine_t
fsm_def_machine_create(const char * name, mem_allocrator_t alloc, error_monitor_t em) {
    size_t name_len = strlen(name) + 1;
    fsm_def_machine_t  m;

    m = mem_alloc(alloc, sizeof(struct fsm_def_machine) + name_len);
    if (m == NULL) return NULL;

    m->m_alloc = alloc;
    m->m_em = em;
    m->m_init_state = NULL;
    m->m_dumper = NULL;

    m->m_state_capacity = 0;
    m->m_state_max = 0;
    m->m_states = NULL;

    memcpy(m + 1, name, name_len);

    if (cpe_hash_table_init(
            &m->m_states_by_name,
            alloc,
            (cpe_hash_fun_t) fsm_def_state_hash,
            (cpe_hash_eq_t) fsm_def_state_eq,
            CPE_HASH_OBJ2ENTRY(fsm_def_state, m_hh),
            -1) != 0)
    {
        mem_free(alloc, m);
        return NULL;
    }

    return m;
}

void fsm_def_machine_free(fsm_def_machine_t m) {
    uint32_t i;

    for(i = 0; i < m->m_state_max + 1; ++i) {
        if (m->m_states[i] == NULL) continue;

        fsm_def_state_free(m->m_states[i]);
        assert(m->m_states[i] == NULL);
    }

    if (m->m_states) mem_free(m->m_alloc, m->m_states);

    cpe_hash_table_fini(&m->m_states_by_name);

    mem_free(m->m_alloc, m);
}

const char * fsm_def_machine_name(fsm_def_machine_t m) {
    return (const char *)(m + 1);
}

int fsm_def_machine_set_init_state(fsm_def_machine_t m, const char * init_state) {
    fsm_def_state_t s = fsm_def_state_find_by_name(m, init_state);
    if (s == NULL) {
        CPE_ERROR(
            m->m_em, "%s: set init state to %s: state is unknown!",
            fsm_def_machine_name(m), init_state);
        return -1;
    }

    m->m_init_state = s;
    return 0;
}

void fsm_def_machine_set_evt_dumper(fsm_def_machine_t m, fsm_evt_dumper_t dumper) {
    m->m_dumper = dumper;
}
