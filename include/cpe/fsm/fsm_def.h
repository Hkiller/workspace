#ifndef CPE_FSM_DEF_H
#define CPE_FSM_DEF_H
#include "cpe/utils/memory.h"
#include "cpe/utils/error.h"
#include "fsm_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*machine def*/
fsm_def_machine_t fsm_def_machine_create(const char * name, mem_allocrator_t alloc, error_monitor_t em);
void fsm_def_machine_free(fsm_def_machine_t m);
const char * fsm_def_machine_name(fsm_def_machine_t m);
int fsm_def_machine_set_init_state(fsm_def_machine_t m, const char * init_state);
void fsm_def_machine_set_evt_dumper(fsm_def_machine_t m, fsm_evt_dumper_t dumper);


/*state def*/
fsm_def_state_t fsm_def_state_create(fsm_def_machine_t m, const char * name);
fsm_def_state_t fsm_def_state_create_ex(fsm_def_machine_t m, const char * name, int state_id);
void fsm_def_state_free(fsm_def_state_t s);

const char * fsm_def_state_name(fsm_def_state_t s);
uint32_t fsm_def_state_id(fsm_def_state_t s);

void fsm_def_state_set_action(
    fsm_def_state_t s,
    fsm_machine_action_t enter,
    fsm_machine_action_t leave);

void fsm_def_state_set_timeout(
    fsm_def_state_t s,
    tl_time_t span,
    fsm_machine_action_t timeout);

int fsm_def_state_add_transition(
    fsm_def_state_t s,
    fsm_def_transition_t transition);

fsm_def_state_t fsm_def_state_find_by_name(fsm_def_machine_t m, const char * name);
fsm_def_state_t fsm_def_state_find_by_id(fsm_def_machine_t m, uint32_t id);

uint32_t fsm_def_state_to_id(fsm_def_machine_t m, const char * state_name);

#ifdef __cplusplus
}
#endif

#endif
