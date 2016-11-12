#ifndef CPE_FSM_TYPES_H
#define CPE_FSM_TYPES_H
#include "cpe/pal/pal_types.h"
#include "cpe/utils/error.h"
#include "cpe/utils/stream.h"
#include "cpe/tl/tl_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*fsm define*/
typedef struct fsm_def_machine * fsm_def_machine_t;
typedef struct fsm_def_state * fsm_def_state_t;

/*fsm instance*/
typedef struct fsm_machine * fsm_machine_t;

typedef void (*fsm_machine_action_t)(fsm_machine_t fsm_ins, fsm_def_state_t state, void * event);

typedef uint32_t (*fsm_def_transition_t)(fsm_machine_t fsm_ins, fsm_def_state_t state, void * event);

typedef void(*fsm_machine_monitor_t)(fsm_machine_t fsm_ins, void * ctx);

typedef void (*fsm_evt_dumper_t)(write_stream_t s, fsm_def_machine_t m, void * event);

#define FSM_INVALID_STATE ((uint32_t)-1)
#define FSM_KEEP_STATE ((uint32_t)-2)
#define FSM_DESTORIED_STATE ((uint32_t)-3)

#ifdef __cplusplus
}
#endif

#endif
