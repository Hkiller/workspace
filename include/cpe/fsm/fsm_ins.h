#ifndef CPE_FSM_INS_H
#define CPE_FSM_INS_H
#include "cpe/utils/memory.h"
#include "cpe/utils/error.h"
#include "fsm_types.h"

#ifdef __cplusplus
extern "C" {
#endif

struct fsm_machine {
    fsm_def_machine_t m_def;
    void * m_ctx;
    int m_debug;
    int32_t m_curent_state;
    void * m_curent_state_ctx;
    void * m_monitors;
};

int fsm_machine_init(fsm_machine_t fsm, fsm_def_machine_t fsm_def, const char * init_state, void * ctx, int debug);
void fsm_machine_fini(fsm_machine_t fsm);
void * fsm_machine_context(fsm_machine_t fsm);
void fsm_machine_set_debug(fsm_machine_t fsm, int debug);
int fsm_machine_curent_state(fsm_machine_t fsm);

int fsm_machine_monitor_add(fsm_machine_t fsm, fsm_machine_monitor_t process, void * process_ctx);
void fsm_machine_monitor_remove(fsm_machine_t fsm, fsm_machine_monitor_t process, void * process_ctx);

void * fsm_state_set_context(fsm_machine_t fsm);
void * fsm_state_context(fsm_machine_t fsm);

int fsm_machine_apply_event(fsm_machine_t fsm, void * evt);

#define fsm_state_to_id(__fsm, __name) fsm_def_state_to_id((__fsm)->m_def, __name)

#ifdef __cplusplus
}
#endif

#endif
