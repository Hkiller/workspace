#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/fsm/fsm_def.h"
#include "fsm_internal_types.h"

fsm_def_state_t fsm_def_state_create(fsm_def_machine_t m, const char * name) {
    return fsm_def_state_create_ex(m, name, m->m_state_max + 1);
}

fsm_def_state_t fsm_def_state_create_ex(fsm_def_machine_t m, const char * name, int id) {
    fsm_def_state_t s;
    size_t name_len = cpe_hs_len_to_binary_len(strlen(name));

    if (id < 0 || id > 2048) {
        CPE_ERROR(m->m_em, "%s: fsm_def_state_create %s(%d): id overflow!", fsm_def_machine_name(m), name, id);
        return NULL;
    }

    if (id + 1 >= m->m_state_capacity) {
        uint32_t new_capacity;
        fsm_def_state_t * new_states;

        new_capacity = m->m_state_capacity;
        do {
            if (new_capacity < 16) {
                new_capacity = 16;
            }
            else {
                new_capacity = new_capacity * 2;
            }
        }
        while(id + 1 >= new_capacity);

        new_states = mem_alloc(m->m_alloc, sizeof(fsm_def_state_t) * new_capacity);
        if (new_states == NULL) {
            CPE_ERROR(m->m_em, "%s: fsm_def_state_create %s: malloc states buf fail!", fsm_def_machine_name(m), name);
            return NULL;
        }

        if (m->m_states) {
            memcpy(new_states, m->m_states, sizeof(fsm_def_state_t) * (m->m_state_max + 1));
            bzero(new_states + m->m_state_max + 1, sizeof(fsm_def_state_t) * (new_capacity - m->m_state_max - 1));
            mem_free(m->m_alloc, m->m_states);
        }
        else {
            bzero(new_states, sizeof(fsm_def_state_t) * new_capacity);
        }

        m->m_states = new_states;
        m->m_state_capacity = new_capacity;
    }

    if (m->m_states[id] != NULL) {
        CPE_ERROR(
            m->m_em, "%s: fsm_def_state_create %s(%d): state id duplicate, old state is %s!",
            fsm_def_machine_name(m), name, id, fsm_def_state_name(m->m_states[id]));
        return NULL;
    }

    s = mem_alloc(m->m_alloc, sizeof(struct fsm_def_state) + name_len);
    if (s == NULL) {
        CPE_ERROR(m->m_em, "%s: fsm_def_state_create %s: alloc state fail!", fsm_def_machine_name(m), name);
        return NULL;
    }

    cpe_hs_init((cpe_hash_string_t)(s + 1), name_len, name);
    s->m_machine = m;
    s->m_id = id;
    s->m_name = (cpe_hash_string_t)(s + 1);
    s->m_enter = NULL;
    s->m_leave = NULL;
    cpe_hash_entry_init(&s->m_hh);

    s->m_trans_count = 0;
    s->m_ext_trans_capacity = 0;
    s->m_ext_trans = NULL;

    if (cpe_hash_table_insert_unique(&m->m_states_by_name, s) != 0) {
        CPE_ERROR(m->m_em, "%s: fsm_def_state_create %s: state name duplicate!", fsm_def_machine_name(m), name);
        mem_free(m->m_alloc, s);
        return NULL;
    }

    m->m_states[id] = s;

    if (id > m->m_state_max) m->m_state_max = id;

    return s;
}

void fsm_def_state_free(fsm_def_state_t s) {
    fsm_def_machine_t m = s->m_machine;

    assert(m);
    assert(s->m_id <= m->m_state_max);
    assert(m->m_states);
    assert(m->m_states[s->m_id] == s);

    m->m_states[s->m_id] = NULL;
    cpe_hash_table_remove_by_ins(&m->m_states_by_name, s);

    if (s->m_ext_trans) {
        mem_free(m->m_alloc, s->m_ext_trans);
    }

    mem_free(m->m_alloc, s);
}

const char * fsm_def_state_name(fsm_def_state_t s) {
    return cpe_hs_data(s->m_name);
}

uint32_t fsm_def_state_id(fsm_def_state_t s) {
    return s->m_id;
}

void fsm_def_state_set_action(fsm_def_state_t s, fsm_machine_action_t enter, fsm_machine_action_t leave) {
    s->m_enter = enter;
    s->m_leave = leave;
}

int fsm_def_state_add_transition(
    fsm_def_state_t s,
    fsm_def_transition_t transition)
{
    if (s->m_trans_count > (sizeof(s->m_base_trans) / sizeof(s->m_base_trans[0]))) {
        int ext_idx = (sizeof(s->m_base_trans) / sizeof(s->m_base_trans[0])) - s->m_trans_count;
        if (ext_idx > s->m_ext_trans_capacity) {
            int new_capacity =
                s->m_ext_trans_capacity < 16
                ? 16
                : s->m_ext_trans_capacity * 2;
            fsm_def_transition_t * new_buf = mem_alloc(s->m_machine->m_alloc, sizeof(fsm_def_transition_t) * new_capacity);
            if (new_buf == NULL) {
                CPE_ERROR(
                    s->m_machine->m_em, "%s: %s add trans: malloc trans buf fail!",
                    fsm_def_machine_name(s->m_machine),
                    fsm_def_state_name(s));
                return -1;
            }

            if (s->m_ext_trans) {
                memcpy(new_buf, s->m_ext_trans, sizeof(fsm_def_transition_t) * s->m_ext_trans_capacity);
                mem_free(s->m_machine->m_alloc, s->m_ext_trans);
            }

            s->m_ext_trans = new_buf;
            s->m_ext_trans_capacity = new_capacity;
        }

        s->m_ext_trans[ext_idx] = transition;
        ++s->m_trans_count;
        return 0;
    }
    else {
        s->m_base_trans[s->m_trans_count] = transition;
        ++s->m_trans_count;
        return 0;
    }
}

fsm_def_state_t fsm_def_state_find_by_name(fsm_def_machine_t m, const char * name) {
    char name_buf[128];
    struct fsm_def_state key;

    cpe_hs_init((cpe_hash_string_t)name_buf, sizeof(name_buf), name);
    key.m_name = (cpe_hash_string_t)name_buf;

    return cpe_hash_table_find(&m->m_states_by_name, &key);
}

fsm_def_state_t fsm_def_state_find_by_id(fsm_def_machine_t m, uint32_t id) {
    if (id > m->m_state_max) return NULL;
    return m->m_states[id];
}

uint32_t fsm_def_state_hash(fsm_def_state_t data) {
    return cpe_hs_value(data->m_name);
}

int fsm_def_state_eq(const fsm_def_state_t l, const fsm_def_state_t r) {
    return cpe_hs_cmp(l->m_name, r->m_name) == 0;
}

uint32_t fsm_def_state_to_id(fsm_def_machine_t m, const char * state_name) {
    fsm_def_state_t s = fsm_def_state_find_by_name(m, state_name);
    return s ? s->m_id : FSM_INVALID_STATE;
}
