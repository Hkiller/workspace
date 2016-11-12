#include <assert.h>
#include "cpe/utils/buffer.h"
#include "cpe/utils/stream_buffer.h"
#include "cpe/fsm/fsm_ins.h"
#include "cpe/fsm/fsm_def.h"
#include "fsm_internal_ops.h"

int fsm_machine_init_ex(fsm_machine_t fsm, fsm_def_machine_t fsm_def, fsm_def_state_t init_state, void * ctx, int debug) {
    fsm->m_def = fsm_def;
    fsm->m_curent_state = fsm_def_state_id(init_state);
    fsm->m_ctx = ctx;
    fsm->m_curent_state_ctx = NULL;
    fsm->m_debug = debug ? 1 : 0;
    fsm->m_monitors = NULL;

    if (fsm->m_debug) {
        CPE_INFO(fsm_def->m_em, "%s(%p): init", fsm_def_machine_name(fsm_def), fsm);
        CPE_INFO(fsm_def->m_em, "%s(%p): state %s: enter", fsm_def_machine_name(fsm_def), fsm, fsm_def_state_name(init_state));
    }

    if (init_state->m_enter) init_state->m_enter(fsm, init_state, NULL);

    return 0;
}

int fsm_machine_init(fsm_machine_t fsm, fsm_def_machine_t fsm_def, const char * init_state_name, void * ctx, int debug) {
    fsm_def_state_t init_state;
    if (init_state_name) {
        init_state = fsm_def_state_find_by_name(fsm_def, init_state_name);
        if (init_state == NULL) {
            CPE_ERROR(
                fsm_def->m_em, "fsm %s no state %s!",
                fsm_def_machine_name(fsm_def),
                init_state_name);
            return -1;
        }
    }
    else {
        init_state = fsm_def->m_init_state;
        if (init_state == NULL) {
            CPE_ERROR(
                fsm_def->m_em, "fsm %s no init state!",
                fsm_def_machine_name(fsm_def));
            return -1;
        }
    }

    return fsm_machine_init_ex(fsm, fsm_def, init_state, ctx, debug);
}

void fsm_machine_fini(fsm_machine_t fsm) {
    fsm_def_state_t cur_state;
 
    assert(fsm->m_def);

    cur_state = fsm_def_state_find_by_id(fsm->m_def, fsm->m_curent_state);
    if (cur_state == NULL) {
        CPE_ERROR(
            fsm->m_def->m_em, "%s(%p): fini: curent state %d not exist!",
            fsm_def_machine_name(fsm->m_def), fsm, fsm->m_curent_state);
    }
    else {
        if (fsm->m_debug) {
            CPE_INFO(
                fsm->m_def->m_em, "%s(%p): state %s: leave",
                fsm_def_machine_name(fsm->m_def), fsm, fsm_def_state_name(cur_state));   
        }
        if (cur_state->m_leave) cur_state->m_leave(fsm, cur_state, NULL);
    }
    
    if (fsm->m_debug) {
        CPE_INFO(fsm->m_def->m_em, "%s(%p): fini", fsm_def_machine_name(fsm->m_def), fsm);
    }

    while(fsm->m_monitors) {
        struct fsm_machine_monitor_node * node = fsm->m_monitors;
        fsm->m_monitors = node->m_next;
        mem_free(fsm->m_def->m_alloc, node);
    }

    fsm->m_def = NULL;
    fsm->m_curent_state = FSM_INVALID_STATE;
    fsm->m_ctx = NULL;
    fsm->m_curent_state_ctx = NULL;
    fsm->m_debug = 0;
}

int fsm_machine_curent_state(fsm_machine_t fsm) {
    return fsm->m_curent_state;
}

void * fsm_machine_context(fsm_machine_t fsm) {
    return fsm->m_ctx;
}

void fsm_machine_set_debug(fsm_machine_t fsm, int debug) {
    fsm->m_debug = debug ? 1 : 0;
}

static const char * fsm_machine_dump_event(fsm_def_machine_t fsm_def, void * evt, mem_buffer_t buff) {
    struct write_stream_buffer s = CPE_WRITE_STREAM_BUFFER_INITIALIZER(buff);

    mem_buffer_init(buff, fsm_def->m_alloc);
    fsm_def->m_dumper((write_stream_t)&s, fsm_def, evt);
    stream_putc((write_stream_t)&s, 0);

    return mem_buffer_make_continuous(buff, 0);
}

int fsm_machine_apply_event(fsm_machine_t fsm, void * evt) {
    fsm_def_machine_t fsm_def;
    fsm_def_state_t cur_state;
    fsm_def_state_t next_state;
    uint32_t next_state_id = FSM_INVALID_STATE;
    int i;
    int base_trans_count;
    struct fsm_machine_monitor_node * monitor;

    fsm_def = fsm->m_def;

    cur_state = fsm_def_state_find_by_id(fsm_def, fsm->m_curent_state);
    if (cur_state == NULL) {
        if (fsm_def->m_dumper) {
            struct mem_buffer buff;
            CPE_ERROR(
                fsm_def->m_em, "%s(%p): apply event %s: curent state %d not exist!",
                fsm_def_machine_name(fsm_def), fsm, fsm_machine_dump_event(fsm_def, evt, &buff), fsm->m_curent_state);
            mem_buffer_clear(&buff);
        }
        else {
            CPE_ERROR(
                fsm_def->m_em, "%s(%p): apply event: curent state %d not exist!",
                fsm_def_machine_name(fsm_def), fsm, fsm->m_curent_state);
        }
        return -1;
    }

    base_trans_count = sizeof(cur_state->m_base_trans) / sizeof(cur_state->m_base_trans[0]);
    if (base_trans_count > cur_state->m_trans_count) base_trans_count = cur_state->m_trans_count;
    for(i = 0; next_state_id== FSM_INVALID_STATE && i < base_trans_count; ++i) {
        next_state_id = cur_state->m_base_trans[i](fsm, cur_state, evt);
    }

    if (next_state_id == FSM_INVALID_STATE
        && cur_state->m_trans_count > (sizeof(cur_state->m_base_trans) / sizeof(cur_state->m_base_trans[0])))
    {
        int ext_trans_count = (sizeof(cur_state->m_base_trans) / sizeof(cur_state->m_base_trans[0])) - cur_state->m_trans_count;
        assert(ext_trans_count <= cur_state->m_ext_trans_capacity);
        assert(cur_state->m_ext_trans);
        
        for(i = 0; next_state_id== FSM_INVALID_STATE && i < base_trans_count; ++i) {
            next_state_id = cur_state->m_ext_trans[i](fsm, cur_state, evt);
        }
    }

    if (next_state_id == FSM_DESTORIED_STATE) goto COMPLETE;

    if (next_state_id == FSM_KEEP_STATE) {
        if (fsm->m_debug) {
            if (fsm_def->m_dumper) {
                struct mem_buffer buff;
                CPE_INFO(
                    fsm_def->m_em, "%s(%p): state %s: process event %s ok, no state change!",
                    fsm_def_machine_name(fsm_def), fsm, fsm_def_state_name(cur_state),
                    fsm_machine_dump_event(fsm_def, evt, &buff));
                mem_buffer_clear(&buff);
            }
            else {
                CPE_INFO(
                    fsm_def->m_em, "%s(%p): state %s: process event ok, no state change!",
                    fsm_def_machine_name(fsm_def), fsm, fsm_def_state_name(cur_state));
            }
        }
        goto COMPLETE;
    }

    if (next_state_id == FSM_INVALID_STATE) {
        if (fsm->m_debug) {
            if (fsm_def->m_dumper) {
                struct mem_buffer buff;
                CPE_INFO(
                    fsm_def->m_em, "%s(%p): state %s: no trans process event %s, trans count = %d!",
                    fsm_def_machine_name(fsm_def), fsm, fsm_def_state_name(cur_state),
                    fsm_machine_dump_event(fsm_def, evt, &buff), cur_state->m_trans_count);
                mem_buffer_clear(&buff);
            }
            else {
                CPE_INFO(
                    fsm_def->m_em, "%s(%p): state %s: no trans process event, trans count = %d!",
                    fsm_def_machine_name(fsm_def), fsm, fsm_def_state_name(cur_state), cur_state->m_trans_count);
            }
        }
        return -1;
    }

    next_state = fsm_def_state_find_by_id(fsm_def, next_state_id);
    if (next_state == NULL) {
        if (fsm_def->m_dumper) {
            struct mem_buffer buff;
            CPE_ERROR(
                fsm_def->m_em, "%s(%p): state %s: apply event %s: next state %d not exist!",
                fsm_def_machine_name(fsm_def), fsm, fsm_def_state_name(cur_state),
                fsm_machine_dump_event(fsm_def, evt, &buff), next_state_id);
            mem_buffer_clear(&buff);
        }
        else {
            CPE_ERROR(
                fsm_def->m_em, "%s(%p): state %s: apply event: next state %d not exist!",
                fsm_def_machine_name(fsm_def), fsm, fsm_def_state_name(cur_state), next_state_id);
        }
        return -1;
    }

    if (fsm->m_debug) {
        if (fsm_def->m_dumper) {
            struct mem_buffer buff;
            CPE_INFO(
                fsm_def->m_em, "%s(%p): state %s: apply event %s: next state is %s(%d)!",
                fsm_def_machine_name(fsm_def), fsm, fsm_def_state_name(cur_state),
                fsm_machine_dump_event(fsm_def, evt, &buff),
                fsm_def_state_name(next_state), next_state_id);
            mem_buffer_clear(&buff);
        }
        else {
            CPE_INFO(
                fsm_def->m_em, "%s(%p): state %s: apply event: next state is %s(%d)!",
                fsm_def_machine_name(fsm_def), fsm, fsm_def_state_name(cur_state),
                fsm_def_state_name(next_state), next_state_id);
        }
    }

    if (fsm->m_debug) {
        CPE_INFO(
            fsm_def->m_em, "%s(%p): state %s: leave",
            fsm_def_machine_name(fsm_def), fsm, fsm_def_state_name(cur_state));
    }
    if (cur_state->m_leave) cur_state->m_leave(fsm, cur_state, NULL);

    fsm->m_curent_state = next_state_id;
    fsm->m_curent_state_ctx = NULL;

    if (fsm->m_debug) {
        CPE_INFO(
            fsm_def->m_em, "%s(%p): state %s: enter",
            fsm_def_machine_name(fsm_def), fsm, fsm_def_state_name(next_state));
    }
    if (next_state->m_enter) next_state->m_enter(fsm, cur_state, evt);

COMPLETE:
    monitor = fsm->m_monitors;
    while(monitor) {
        monitor->m_process(fsm, monitor->m_process_ctx);
        monitor = monitor->m_next;
    }

    return 0;
}

int fsm_machine_monitor_add(fsm_machine_t fsm, fsm_machine_monitor_t process, void * process_ctx) {
    struct fsm_machine_monitor_node ** head = (struct fsm_machine_monitor_node **)&fsm->m_monitors;
    struct fsm_machine_monitor_node * new_node = mem_alloc(fsm->m_def->m_alloc, sizeof(struct fsm_machine_monitor_node));
    if (new_node == NULL) return -1;

    new_node->m_next = 0;
    new_node->m_process = process;
    new_node->m_process_ctx = process_ctx;

    while(*head) head = &(*head)->m_next;

    *head = new_node;

    return 0;
}

void fsm_machine_monitor_remove(fsm_machine_t fsm, fsm_machine_monitor_t process, void * process_ctx) {
    struct fsm_machine_monitor_node ** head = (struct fsm_machine_monitor_node **)&fsm->m_monitors;

    while(*head) {
        struct fsm_machine_monitor_node * check = *head;
        
        if (check->m_process_ctx == process_ctx && check->m_process == process) {
            *head = check->m_next;
            mem_free(fsm->m_def->m_alloc, check);
        }
        else {
            head = &check->m_next;
        }
    }
}
