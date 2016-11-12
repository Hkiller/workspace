#ifndef CPE_FSM_INTERNAL_OPS_H
#define CPE_FSM_INTERNAL_OPS_H
#include "fsm_internal_types.h"

uint32_t fsm_def_state_hash(fsm_def_state_t data);
int fsm_def_state_eq(const fsm_def_state_t l, const fsm_def_state_t r);

#endif
