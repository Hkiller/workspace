#ifndef GD_LOG_INTERNAL_OPS_H
#define GD_LOG_INTERNAL_OPS_H
#include "cpe/cfg/cfg_types.h"
#include "log_internal_types.h"

struct log_context * log_context_create(gd_app_context_t app, error_monitor_t em, mem_allocrator_t alloc);
struct log_context * log_context_find(gd_app_context_t app);
void log_context_free(struct log_context * mgr);
const char * log_context_name(struct log_context * ctx);

int log4c_em_create(struct log_context * context, const char * em_name, log4c_category_t * category);
void log4c_em_free_all(struct log_context * context);

#endif
