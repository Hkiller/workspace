#ifndef CPE_TL_ACTION_H
#define CPE_TL_ACTION_H
#include "cpe/utils/memory.h"
#include "tl_types.h"

#ifdef __cplusplus
extern "C" {
#endif

tl_event_t tl_event_create(tl_t tl, size_t dataSize);
tl_event_t tl_event_clone(tl_event_t e, mem_allocrator_t alloc);
void tl_event_free(tl_event_t e);

void * tl_event_data(tl_event_t event);
size_t tl_event_capacity(tl_event_t event);
tl_t tl_event_tl(tl_event_t event);
int tl_event_in_queue(tl_event_t event);
tl_event_t tl_event_from_data(void * data);

tl_event_t tl_action_add(tl_t tl);

/* repeatCount: >= 1 execute fix count
                < 0  repeat no limit
 */
int tl_event_send_ex(
    tl_event_t event,
    tl_time_span_t delay,
    tl_time_span_t span,
    int repeatCount);

#ifdef __cplusplus
}
#endif

#endif
