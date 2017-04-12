#ifndef USF_LOGIC_QUEUE_H
#define USF_LOGIC_QUEUE_H
#include "cpe/utils/hash_string.h"
#include "logic_types.h"

#ifdef __cplusplus
extern "C" {
#endif

logic_queue_t
logic_queue_create(logic_manage_t manage, const char * name);
void logic_queue_free(logic_queue_t queue);

logic_queue_t
logic_queue_find(logic_manage_t mgr, cpe_hash_string_t queue_name);

const char * logic_queue_name(logic_queue_t queue);
cpe_hash_string_t logic_queue_name_hs(logic_queue_t queue);

uint32_t logic_queue_count(logic_queue_t queue);
uint32_t logic_queue_max_count(logic_queue_t queue);
void logic_queue_set_max_count(logic_queue_t queue, uint32_t max_count);

logic_context_t logic_queue_head(logic_queue_t queue);

int logic_queue_enqueue_head(logic_queue_t queue, logic_context_t context);
int logic_queue_enqueue_tail(logic_queue_t queue, logic_context_t context);
int logic_queue_enqueue_after(logic_context_t pre, logic_context_t context);

void logic_queue_dequeue(logic_queue_t queue, logic_context_t context);

#ifdef __cplusplus
}
#endif

#endif

